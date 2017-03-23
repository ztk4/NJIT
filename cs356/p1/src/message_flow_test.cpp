#include "message_flow.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_timeout.h"
#include "mock_message_io.h"

using namespace std;
using ::testing::Exactly;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::_;
using util::Timeout;
using util::TimeoutFactory;
using util::MockTimeout;
using util::MockTimeoutFactory;

namespace router {
namespace {
const uint16_t kRouterId = 0x4242;

class ClientTest : public ::testing::Test {
 protected:
  static void TearDownTestCase() {
    ResetAll();
  }

  ClientTest()
      : mock_message_io_factory_(new MockMessageIoFactory),
        mock_message_io_(new MockMessageIo),
        mock_timeout_factory_(new MockTimeoutFactory),
        mock_timeout_(new MockTimeout),
        client_(mock_message_io_factory_, mock_timeout_factory_) {}

  // The following are NOT OWNED
  MockMessageIoFactory *mock_message_io_factory_;
  MockMessageIo *mock_message_io_;
  MockTimeoutFactory *mock_timeout_factory_;
  MockTimeout *mock_timeout_;
  // The following ARE owned
  Client client_;
};

TEST_F(ClientTest, SendGetTableSendsWellFormedMessageExactlyOnce) {
  // Set expectations on Factories
  EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
    .WillOnce(Return(mock_message_io_));
  EXPECT_CALL(*mock_timeout_factory_, MakeTimeout())
    .WillOnce(Return(mock_timeout_));

  Message m(0, Message::UNKNOWN, 0);
  // Set expectations on other mocks
  EXPECT_CALL(*mock_message_io_, SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Return(true)));
  EXPECT_CALL(*mock_timeout_, Start(_));

  // Send request.
  client_.SendGetTableRequest(kRouterId);

  // Check message
  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::REQUEST_TABLE);
  EXPECT_TRUE(m.Table().empty());
}

TEST_F(ClientTest, SendGetTableSendMessageEachTimeTimeoutCallsBack) {
  const int kTrials = 5;
  // Set expectations on Factories
  EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
    .WillOnce(Return(mock_message_io_));
  EXPECT_CALL(*mock_timeout_factory_, MakeTimeout())
    .WillOnce(Return(mock_timeout_));

  function<void(void)> callback;
  // Set expectations on other mocks
  EXPECT_CALL(*mock_message_io_, SendTo(_, _))
    .Times(Exactly(kTrials))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(*mock_timeout_, Start(_))
    .Times(Exactly(kTrials))
    .WillRepeatedly(SaveArg<0>(&callback));

  // Send request
  client_.SendGetTableRequest(kRouterId);

  // Start at 1 because it sends once immediately.
  for (int i = 1; i < kTrials; ++i)
    callback();
}
}  // anonymous namespace
}  // namespace router
