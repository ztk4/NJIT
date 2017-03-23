#ifndef ROUTER_MOCK_MESSAGE_IO_H_
#define ROUTER_MOCK_MESSAGE_IO_H_

#include "message_io.h"

#include "gmock/gmock.h"

namespace router {
/// Mock implementation of MessageIo
class MockMessageIo : public MessageIo {
 public:
  ~MockMessageIo() override = default;

  MOCK_METHOD2(SendTo, bool(const Message &m, uint16_t router_id));
  MOCK_METHOD2(ReceiveFrom, Message(uint16_t &router_id, bool &status));
};
}  // namespace router

#endif  // ROUTER_MOCK_MESSAGE_IO_H_
