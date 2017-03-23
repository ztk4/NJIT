#include "message_flow.h"

#include <mutex>
#include <thread>
#include <utility>

using namespace std;
using util::Timeout;
using util::TimeoutFactory;

namespace router {
namespace {
// RepeatedSend object for repeatedly sending on timeout if needed.
class RepeatedSend {
 public:
  RepeatedSend(uint16_t router_id, const Message &m,
      unique_ptr<MessageIo> &&message_io, unique_ptr<Timeout> &&timeout)
      : router_id_(router_id),
        m_(m),
        message_io_(move(message_io)),
        timeout_(move(timeout)) {
    Send();      
  }
  RepeatedSend(const RepeatedSend &) = delete;
  RepeatedSend(RepeatedSend &&) = delete;
  RepeatedSend &operator=(const RepeatedSend &) = delete;
  RepeatedSend &operator=(RepeatedSend &&) = delete;

  bool IsActive() const { return static_cast<bool>(timeout_); }

  void Send() {
    message_io_->SendTo(m_, router_id_);
    if (timeout_) {
      timeout_->Start([this]() { Send(); });
    }
  }

 private:
  uint16_t router_id_;
  Message m_;
  unique_ptr<MessageIo> message_io_;
  unique_ptr<Timeout> timeout_;
};

// Keeps track of last used identifier
atomic<uint16_t> last_used_id(0);
// Maps message ids to RepeatedSend objects relevant to that id.
map<uint16_t, RepeatedSend> sending;
// Mutex for guarding the timeouts map.
mutex sending_mutex;  // GUARDS sending.

// Internal implementation of send to. Will send m to router_id using
// message_io at least once. If timeout is non-null, a RepeatedSend object will
// be put into the sending map waiting for a response. This will repeatedly
// send requests on timeout and should be deleted with the corresponding
// response is received.
void InternalSendTo(uint16_t router_id, const Message &m,
    unique_ptr<MessageIo> &&message_io,
    unique_ptr<Timeout> &&timeout = move(unique_ptr<Timeout>())) {
  lock_guard<mutex> sending_mutex_lock(sending_mutex);
  sending.erase(m.SourceId());  // Erase existing pair if needed.
  // Build RepeatedSend in map.
  sending.emplace(piecewise_construct, forward_as_tuple(m.SourceId()),
      forward_as_tuple(router_id, m, move(message_io), move(timeout)));
}
}  // anonymous namespace
void ResetAll() {
  lock_guard<mutex> sending_mutex_lock(sending_mutex);
  for (auto it = sending.begin(); it != sending.end(); ++it)
    sending.erase(it);
}

// Client Implementation
Client::Client(MessageIoFactory *message_io_factory,
    TimeoutFactory *timeout_factory)
    : message_io_factory_(message_io_factory),
      timeout_factory_(timeout_factory) {}

void Client::SendGetTableRequest(uint16_t router_id) {
  Message m(0, Message::REQUEST_TABLE, ++last_used_id);

  InternalSendTo(router_id, m,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())),
      move(unique_ptr<Timeout>(timeout_factory_->MakeTimeout())));
}

void Client::PushTableTo(uint16_t router_id,
    const map<uint16_t, int16_t> &table) {
  Message m(0, Message::PUSH_TABLE, ++last_used_id, table);

  InternalSendTo(router_id, m,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())),
      move(unique_ptr<Timeout>(timeout_factory_->MakeTimeout())));
}

// Server Implementation
Server::Server(MessageIoFactory *message_io_factory,
    TimeoutFactory *timeout_factory)
    : message_io_factory_(message_io_factory),
      timeout_factory_(timeout_factory),
      active_(true),
      alive_(true) {
  // Claim ownership of a single message_io
  MessageIo *message_io = message_io_factory->MakeMessageIo();
  
  // Spawn thread that waits on incoming connections
  thread([this, message_io]() {
      // Thread claims ownership of transferred message_io
      unique_ptr<MessageIo> message_io_deleter(message_io);
      // While Server is active
      while (active_) {
        uint16_t router_id;
        bool status;
        Message m = message_io->ReceiveFrom(router_id, status);

        if (status) {
          switch (m.GetType()) {
            case Message::REQUEST_TABLE:
              GetRequestReceived(router_id, m);
              break;
            case Message::PUSH_TABLE:
              PushTableReceived(router_id, m);
              break;
            case Message::TABLE_RESPONSE:
              TableResponseReceived(router_id, m);
              break;
            case Message::ACK:
              AckReceived(router_id, m);
              break;

            case Message::UNKNOWN:
              break;
          }
        }

        this_thread::yield();
      }
  
      // Notify server that thread is about to terminate.
      alive_ = false;
    }).detach();  // Detach thread.
}

Server::~Server() {
  // Notify thread the server is going down.
  active_ = false;
  
  // Wait on thread before invalidating captured pointer.
  while (alive_)
    this_thread::yield();
}

void Server::OnTableReceipt(
    const function<void(const map<uint16_t, int16_t> &)> &callback) {
  table_receipt_ = callback;
}

void Server::OnTableRequest(
    const function<const map<uint16_t, int16_t> &(void)> &callback) {
  table_request_ = callback;
}

void Server::GetRequestReceived(uint16_t router_id, const Message &m) {
  map<uint16_t, int16_t> curr_table;
  if (table_request_)
    curr_table = table_request_();

  Message resp(m.SourceId(), Message::TABLE_RESPONSE, ++last_used_id,
      curr_table);

  InternalSendTo(router_id, resp,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())),
      move(unique_ptr<Timeout>(timeout_factory_->MakeTimeout())));
}

void Server::PushTableReceived(uint16_t router_id, const Message &m) {
  Message resp(m.SourceId(), Message::ACK, ++last_used_id);

  InternalSendTo(router_id, resp,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())));

  table_receipt_(m.Table());
}

void Server::TableResponseReceived(uint16_t router_id, const Message &m) {
  {  // Check in sending map for previous message
    lock_guard<mutex> sending_mutex_lock(sending_mutex);
    auto it = sending.find(m.DestinationId());
    if (it == sending.end())
      return;  // Not in map, therefore this is a duplicate response
    sending.erase(it);  // Remove listing (mark receipt and stop sending).
  }

  Message resp(m.SourceId(), Message::ACK, m.DestinationId());

  InternalSendTo(router_id, resp,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())));

  table_receipt_(m.Table());
}

void Server::AckReceived(uint16_t, const Message &m) {
  // Remove previous message from sending map if there
  lock_guard<mutex> sending_mutex_lock(sending_mutex);
  sending.erase(m.DestinationId());
}
}  // namespace router
