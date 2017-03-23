#include "message_io.h"

#include <cstring>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_socket.h"

using namespace std;
using util::Socket;
using util::InSocket;
using util::MockSocket;
using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::Invoke;
using ::testing::Pointee;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::WhenDynamicCastTo;
using ::testing::WithArg;
using ::testing::WithArgs;
using ::testing::_;

namespace router {
namespace {
class SocketMessageIoTest : public ::testing::Test {
 protected:
  static const uint16_t kRouterId;
  static const InSocket::InAddress kAddress;
  // Called before all tests from this fixture.
  static void SetUpTestCase() {
    SocketMessageIo::SetSocket(mock_socket_ = new MockSocket());
    SocketMessageIo::AddRouterAddressPair(kRouterId, kAddress);
  }
  // Called after all tests from this fixture run.
  static void TearDownTestCase() {
    SocketMessageIo::CloseSocket();
  }

  SocketMessageIoTest() 
      : m_(1, Message::ACK, 1),
        message_io_(new SocketMessageIo()) {}

  Message m_;
  unique_ptr<MessageIo> message_io_;
  static MockSocket *mock_socket_;
};

const uint16_t SocketMessageIoTest::kRouterId = 0xABCD;
const InSocket::InAddress SocketMessageIoTest::kAddress(0xDEADBEEF, 0x4242);
MockSocket *SocketMessageIoTest::mock_socket_;

TEST_F(SocketMessageIoTest, SendToFailsWithInvalidMessage) {
  EXPECT_FALSE(message_io_->SendTo(Message(0, Message::UNKNOWN, 0), kRouterId));
}

TEST_F(SocketMessageIoTest, SendToFailsWithUnknownRouterId) {
  EXPECT_FALSE(message_io_->SendTo(m_, kRouterId + 1));
}

TEST_F(SocketMessageIoTest, SendToPassesMessageAlongToSocketWithRightAddress) {
  Message passed_message(0, Message::UNKNOWN, 0);
  // Expects proper address to be passed back.
  EXPECT_CALL(*mock_socket_, SendTo(_, _, _,
        WhenDynamicCastTo<const InSocket::InAddress *>(Pointee(kAddress))))
    // Assigns the deserialization of the passed buffer to passed message
    .WillOnce(DoAll(
          WithArgs<0, 1>(Invoke(
              [&passed_message](const void *buf, size_t len) {
                passed_message = Message::Deserialize(buf, len);
              })),
          Return(0)
        ));

  message_io_->SendTo(m_, kRouterId);

  EXPECT_EQ(passed_message, m_);
}

TEST_F(SocketMessageIoTest, SendToFailsWithBadReturnLength) {
  EXPECT_CALL(*mock_socket_, SendTo(_, _, _, _))
    .WillOnce(Return(0));

  EXPECT_FALSE(message_io_->SendTo(Message(1, Message::ACK, 1), kRouterId));
}

TEST_F(SocketMessageIoTest, SendToSucceedsWithOkReturnLength) {
  EXPECT_CALL(*mock_socket_, SendTo(_, _, _, _))
    .WillOnce(Return(m_.SerializedLength()));

  EXPECT_TRUE(message_io_->SendTo(m_, kRouterId));
}

TEST_F(SocketMessageIoTest, ReceiveFromFailsWithBadReturnLength) {
  uint16_t router_id;
  bool status = true;

  EXPECT_CALL(*mock_socket_, RecvFrom(_, _, _, _))
    .WillOnce(DoAll(
        WithArg<3>(Invoke(
            [](Socket::Address *addr) {
              memcpy(addr->Value(), kAddress.Value(), addr->Length());
            })),
        Return(0)
        ));

  message_io_->ReceiveFrom(router_id, status);
  EXPECT_FALSE(status);
}

TEST_F(SocketMessageIoTest, ReceiveFromFailsWithBadAddress) {
  uint16_t router_id;
  bool status = true;

  EXPECT_CALL(*mock_socket_, RecvFrom(_, _, _, _))
    .WillOnce(DoAll(
          WithArg<3>(Invoke(
              [](Socket::Address *addr) {
                memcpy(addr->Value(), InSocket::InAddress(42, 42).Value(),
                    addr->Length());
              })),
          Return(m_.SerializedLength())
          ));

  message_io_->ReceiveFrom(router_id, status);
  EXPECT_FALSE(status);
}

TEST_F(SocketMessageIoTest, ReceiveFromHasCorrectOutputWithGoodSocket) {
  uint16_t router_id;
  bool status = true;
  char buf[m_.SerializedLength()];
  m_.Serialize(buf, m_.SerializedLength());

  EXPECT_CALL(*mock_socket_, RecvFrom(_, _, _, _))
    .WillOnce(DoAll(
          WithArg<3>(Invoke(
              [](Socket::Address *addr) {
                memcpy(addr->Value(), kAddress.Value(), addr->Length());
              })),
          WithArg<0>(Invoke(
              [&buf, this](void *buffer) {
                memcpy(buffer, buf, m_.SerializedLength());
              })),
          Return(m_.SerializedLength())
          ));

  Message m = message_io_->ReceiveFrom(router_id, status);

  EXPECT_EQ(m, m_);
  EXPECT_TRUE(status);
  EXPECT_EQ(router_id, kRouterId);
}         
}  // namespace anonymous
}  // namespace router
