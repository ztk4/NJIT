#include "message_flow.h"

#include <atomic>
#include <thread>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_timeout.h"
#include "mock_message_io.h"

using namespace std;
using ::testing::Exactly;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::InvokeWithoutArgs;
using ::testing::Mock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgReferee;
using ::testing::WithArg;
using ::testing::_;
using util::Timeout;
using util::TimeoutFactory;
using util::MockTimeout;
using util::MockTimeoutFactory;

namespace router {
namespace {
const int kTrials = 5;
const uint16_t kRouterId = 0x4242;
const uint16_t kSourceId = 0xDEAD;
const uint16_t kDestId = 0xBEEF;
const map<uint16_t, int16_t> kTable = {
  {kRouterId, 128}, {0, 0}, {5, -1}, {0xDEAD, 500}
};

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

TEST_F(ClientTest, NormalRequestsSendOnncePerCallback) {
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

TEST_F(ClientTest, PushTableToSendsWellFormedMessageExactlyOnce) {
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

  // Send request
  client_.PushTableTo(kRouterId, kTable);

  // Check Message
  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::PUSH_TABLE);
  EXPECT_EQ(m.Table(), kTable);
}

class ServerTest : public ::testing::Test {
 protected:
  static void TearDownTestCase() {
    ResetAll();
  }

  ServerTest()
      : mock_message_io_factory_(new MockMessageIoFactory),
        mock_timeout_factory_(new MockTimeoutFactory) {}

  void StartServer() {
    server_.reset(new Server(mock_message_io_factory_, mock_timeout_factory_,
          kTable));

    server_->OnTableReceipt([this](const map<uint16_t, int16_t> &table) {
        table_update_ = table;
      });
  }

  // The following are NOT OWNED
  MockMessageIoFactory *mock_message_io_factory_;
  MockTimeoutFactory *mock_timeout_factory_;
  // The following ARE owned
  map<uint16_t, int16_t> table_update_;
  unique_ptr<Server> server_;
  atomic<bool> wait_;
};

/*
TEST_F(ServerTest, ReceiptOfMatchingRequestCancelsTimeout) {
  MockMessageIo *mock_io1 = new MockMessageIo, *mock_io2 = new MockMessageIo,
                *mock_io3 = new MockMessageIo;
  MockTimeout *timeout = new MockTimeout;

  // Sequence so that mock io's are assigned properly
  {
    InSequence seq;
    EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
      .WillOnce(Return(mock_io1));
    EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
      .WillOnce(Return(mock_io2));
    EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
      .WillOnce(Return(mock_io3));
  }
  EXPECT_CALL(*mock_timeout_factory_, MakeTimeout())
    .WillOnce(Return(timeout));

  // Needed for matching later
  uint16_t source_id;
  EXPECT_CALL(*mock_io1, ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(0, Message::REQUEST_TABLE, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_io2, SendTo(_, kRouterId))
    .WillOnce(DoAll(
          WithArg<0>(Invoke([&source_id](const Message &m) {
                source_id = m.SourceId();
              })),
          InvokeWithoutArgs([this]() { wait_ = false; }),
          Return(true)));
  EXPECT_CALL(*timeout, Start(_));

  wait_ = true;
  StartServer();

  while (wait_)
    this_thread::yield();

  // Clear first mock's expectations so we can inject another message
  Mock::VerifyAndClear(mock_io1);
  EXPECT_CALL(*mock_io1, ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(source_id, Message::ACK, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_io3, SendTo(_, kRouterId))
    .WillOnce(DoAll(
        InvokeWithoutArgs([this]() { wait_ = false; }),
        Return(true)));
  EXPECT_CALL(*timeout, Cancel());

  wait_ = true;
  while(wait_)
    this_thread::yield();
}
*/

TEST_F(ServerTest, ReceiptOfTableRequestResultsInExactlyOneWellFormedMessage) {
  MockMessageIo *mock_io1 = new MockMessageIo, *mock_io2 = new MockMessageIo;
  MockTimeout *timeout = new MockTimeout;

  // Sequence so that io1 is for receival and io2 is for sending
  {
    InSequence seq;
    EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
      .WillOnce(Return(mock_io1));
    EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
      .WillOnce(Return(mock_io2));
  }
  EXPECT_CALL(*mock_timeout_factory_, MakeTimeout())
    .WillOnce(Return(timeout));

  Message m(0, Message::UNKNOWN, 0);
  EXPECT_CALL(*mock_io1, ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(0, Message::REQUEST_TABLE, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_io2, SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          InvokeWithoutArgs([this]() { wait_ = false; }),
          Return(true)));
  EXPECT_CALL(*timeout, Start(_));

  wait_ = true;
  StartServer();

  while (wait_)
    this_thread::yield();

  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::TABLE_RESPONSE);
  EXPECT_EQ(m.DestinationId(), kSourceId);
  EXPECT_EQ(m.Table(), kTable);
}
}  // anonymous namespace
}  // namespace router
