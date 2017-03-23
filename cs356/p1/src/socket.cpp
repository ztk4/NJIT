#include "socket.h"

namespace util {
// InAddress Implementation.
InSocket::InAddress::InAddress(in_addr_t addr, in_port_t port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = port;
  addr_.sin_addr.s_addr = addr;
}

InSocket::InAddress::InAddress(const InSocket::InAddress &addr)
    : InAddress(addr.addr_.sin_addr.s_addr, addr.addr_.sin_port) {}

InSocket::InAddress &InSocket::InAddress::operator=(
    const InSocket::InAddress &addr) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = addr.addr_.sin_port;
  addr_.sin_addr.s_addr = addr.addr_.sin_addr.s_addr;

  return *this;
}

struct sockaddr *InSocket::InAddress::Value() {
  return (struct sockaddr *) &addr_;
}

const struct sockaddr *InSocket::InAddress::Value() const {
  return (const struct sockaddr *) &addr_;
}

socklen_t InSocket::InAddress::Length() const {
  return sizeof(addr_);
}

bool InSocket::InAddress::operator==(const InSocket::InAddress &addr) const {
  return addr_.sin_port == addr.addr_.sin_port &&
    addr_.sin_addr.s_addr == addr.addr_.sin_addr.s_addr;
}

bool InSocket::InAddress::operator!=(const InSocket::InAddress &addr) const {
  return !(*this == addr);
}

size_t InSocket::InAddress::Hash() const {
  return (static_cast<uint64_t>(addr_.sin_addr.s_addr) << 16) | // 32-bits
    static_cast<uint64_t>(addr_.sin_port);  // 16-bits
}

// InSocket Implementation.
InSocket::InSocket(Type type) {
  // Only implements UDP for now ;)
  switch (type) {
    case UDP:
      socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
      break;
  }
}

InSocket::~InSocket() {
  if (IsOpen())
    shutdown(socket_, SHUT_RDWR);
}

bool InSocket::Bind(const Socket::Address *addr) {
  return bind(socket_, addr->Value(), addr->Length()) == 0;
}

ssize_t InSocket::RecvFrom(void *buffer, size_t length, int flags,
    Address *addr) {
  auto addr_len = addr->Length();
  return recvfrom(socket_, buffer, length, flags, addr->Value(), &addr_len);
}

ssize_t InSocket::SendTo(const void *message, size_t length, int flags,
    const Address *addr) {
  return sendto(socket_, message, length, flags, addr->Value(), addr->Length());
}
}  // namespace util