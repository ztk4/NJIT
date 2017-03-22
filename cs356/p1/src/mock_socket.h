#ifndef UTIL_MOCK_SOCKET_H_
#define UTIL_MOCK_SOCKET_H_

#include "socket.h"

#include "gmock/gmock.h"

namespace util {
class MockSocket : public Socket {
 public:
  ~MockSocket() override = default;

  MOCK_METHOD1(Bind, bool(const Address *addr));
  MOCK_METHOD4(RecvFrom, ssize_t(void *buffer, size_t length, int flags,
        Address *addr));
  MOCK_METHOD4(SendTo, ssize_t(const void *message, size_t length, int flags,
        const Address *addr));
  MOCK_CONST_METHOD0(IsOpen, bool());
};
}  // namespace util

#endif  // UTIL_MOCK_SOCKET_H_
