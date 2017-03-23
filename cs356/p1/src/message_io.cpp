#include "message_io.h"

using namespace std;
using util::Socket;
using util::InSocket;

namespace router {
namespace {
const size_t kBufSize = 128;
}  // anonymous namespace
// Static Members of SocketMessageIo
unordered_map<uint16_t, InSocket::InAddress>
    SocketMessageIo::addr_lookup_table_;
unordered_map<InSocket::InAddress, uint16_t>
    SocketMessageIo::rev_lookup_table_;
mutex SocketMessageIo::addr_lookup_table_mutex_;
mutex SocketMessageIo::rev_lookup_table_mutex_;
unique_ptr<Socket> SocketMessageIo::socket_;

bool SocketMessageIo::SendTo(const Message &m, uint16_t router_id) {
  if (!m.IsValid() || !socket_)
    return false;

  // Get Address of router.
  const InSocket::InAddress *addr;
  {
    lock_guard<mutex> addr_lookup_table_mutex_lock(addr_lookup_table_mutex_);
    auto it = addr_lookup_table_.find(router_id);
    if (it == addr_lookup_table_.end())
      return false;
    addr = &(it->second);
  }

  // Serialize message.
  size_t len = m.SerializedLength();
  void *buffer = malloc(len);
  m.Serialize(buffer, len);

  bool ret = socket_->SendTo(buffer, len, 0, addr) > 0;
  free(buffer);
  return ret;
}

Message SocketMessageIo::ReceiveFrom(uint16_t &router_id, bool &status) {
  if (!socket_) {
    status = false;
    return Message(0, Message::UNKNOWN, 0);
  }

  // Receive a packet (blocks)
  char buffer[kBufSize];
  InSocket::InAddress addr;
  ssize_t len = socket_->RecvFrom(buffer, kBufSize, 0, &addr);
  status = len > 0;

  // Get the router_id.
  {
    lock_guard<mutex> rev_lookup_table_mutex_lock(rev_lookup_table_mutex_);
    auto it = rev_lookup_table_.find(addr);
    if (it == rev_lookup_table_.end()) {
      status = false;
      return Message(0, Message::UNKNOWN, 0);
    }
    router_id = it->second;
  }

  // Deserialize and return message.
  return status ? Message::Deserialize(buffer, len) :
    Message(0, Message::UNKNOWN, 0);
}

void SocketMessageIo::AddRouterAddressPair(uint16_t router_id,
    const InSocket::InAddress &addr) {
  {  // Scope for accessing addr_lookup_table_
    lock_guard<mutex> addr_lookup_table_mutex_lock(addr_lookup_table_mutex_);
    addr_lookup_table_[router_id] = addr;
  }
  {  // Scope for accessing rev_lookup_table_
    lock_guard<mutex> rev_lookup_table_mutex_lock(rev_lookup_table_mutex_);
    rev_lookup_table_[addr] = router_id;
  }
}

void SocketMessageIo::SetSocket(Socket *s) {
  socket_.reset(s);  // Deletes old socket.
}

void SocketMessageIo::CloseSocket() {
  socket_.reset();
}
}  // namespace router
