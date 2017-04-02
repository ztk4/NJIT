#include "message_flow.h"

#include <thread>
#include <utility>

using namespace std;
using util::ThreadPool;
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
  // Disallow copy, move, and all assignment.
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
  // Build RepeatedSend in map (nothing should be moved ever).

  if (timeout) {
    // Place in map to send repeatedly until cleaned.
    sending.emplace(piecewise_construct, forward_as_tuple(m.SourceId()),
        forward_as_tuple(router_id, m, move(message_io), move(timeout)));
  } else {
    // If not sending repeatedly, construct once on stack then destruct.
    RepeatedSend(router_id, m, move(message_io), move(timeout));
  }
}
}  // anonymous namespace
// Resets above sending map.
void ResetAll() {
  lock_guard<mutex> sending_mutex_lock(sending_mutex);
  sending.clear();
}

// Client Implementation
Client::Client(MessageIoFactory *message_io_factory,
    TimeoutFactory *timeout_factory, int max_threads)
    : message_io_factory_(message_io_factory),
      timeout_factory_(timeout_factory),
      pool_(max_threads) {}

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

void Client::BroadcastTable(set<uint16_t> neighbor_ids,
    const map<uint16_t, int16_t> &table) {
  // Blocks on completion.
  pool_.Map([this, table](uint16_t router_id) { 
      PushTableTo(router_id, table);
    }, neighbor_ids.begin(), neighbor_ids.end());
}

// Server Implementation
Server::Server(MessageIoFactory *message_io_factory,
    TimeoutFactory *timeout_factory, int max_threads)
    : message_io_factory_(message_io_factory),
      timeout_factory_(timeout_factory),
      pool_(max_threads),
      active_(true) {
  // Dispatch max_threads for server, non-blocking.
  pool_.ExecuteAsync([this]() {
      // Thread claims ownership of transferred message_io
      unique_ptr<MessageIo> message_io(message_io_factory_->MakeMessageIo());
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
    }, max_threads);
}

Server::~Server() {
  // Notify threads the server is going down.
  active_ = false;
  
  // Wait on pool before invalidating captured pointer.
  pool_.JoinAll();
}

void Server::OnTableReceipt(
    const function<void(uint16_t, map<uint16_t, int16_t>)> &callback) {
  lock_guard<mutex> receipt_cb_mutex_lock(receipt_cb_mutex_);
  receipt_cb_ = callback;
}

void Server::OnTableRequest(
    const function<map<uint16_t, int16_t>(void)> &callback) {
  lock_guard<mutex> request_cb_mutex_lock(request_cb_mutex_);
  request_cb_ = callback;
}

void Server::GetRequestReceived(uint16_t router_id, const Message &m) {
  map<uint16_t, int16_t> routing_table;
  {
    lock_guard<mutex> request_cb_mutex_lock(request_cb_mutex_);
    if (request_cb_)
      routing_table = request_cb_();
  }

  Message resp(m.SourceId(), Message::TABLE_RESPONSE, ++last_used_id,
      routing_table);

  InternalSendTo(router_id, resp,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())),
      move(unique_ptr<Timeout>(timeout_factory_->MakeTimeout())));
}

void Server::PushTableReceived(uint16_t router_id, const Message &m) {
  Message resp(m.SourceId(), Message::ACK, ++last_used_id);

  InternalSendTo(router_id, resp,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())));

  {
    lock_guard<mutex> receipt_cb_mutex_lock(receipt_cb_mutex_);
    if (receipt_cb_)
      receipt_cb_(router_id, m.Table());
  }
}

void Server::TableResponseReceived(uint16_t router_id, const Message &m) {
  {  // Check in sending map for previous message
    lock_guard<mutex> sending_mutex_lock(sending_mutex);
    auto it = sending.find(m.DestinationId());
    if (it != sending.end())
      sending.erase(it);  // Remove listing (mark receipt and stop sending).
  }

  Message resp(m.SourceId(), Message::ACK, m.DestinationId());

  InternalSendTo(router_id, resp,
      move(unique_ptr<MessageIo>(message_io_factory_->MakeMessageIo())));

  {
    lock_guard<mutex> receipt_cb_mutex_lock(receipt_cb_mutex_);
    if (receipt_cb_)
      receipt_cb_(router_id, m.Table());
  }
}

void Server::AckReceived(uint16_t, const Message &m) {
  // Remove previous message from sending map if there
  lock_guard<mutex> sending_mutex_lock(sending_mutex);
  sending.erase(m.DestinationId());
}
}  // namespace router
