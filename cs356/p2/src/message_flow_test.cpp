#include "message_flow.h"

#include <atomic>
#include <thread>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <vector>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mock_timeout.h"
#include "mock_message_io.h"

using namespace std;
using ::testing::Assign;
using ::testing::DoAll;
using ::testing::Exactly;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::IgnoreResult;
using ::testing::IsEmpty;
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
const uint16_t kLocalId = 0xAAAA;
const uint16_t kSourceId = 0xDEAD;
const uint16_t kDestId = 0xBEEF;
const map<uint16_t, int16_t> kTable = {
  {kRouterId, 128}, {0, 0}, {5, -1}, {42, 500}
};
const set<uint16_t> kNeighbors = {kRouterId, 5};

class ClientTest : public ::testing::Test {
 protected:
  static void TearDownTestCase() {
    ResetAll();  // Destroys hanging objects.
  }

  ClientTest()
      : mock_message_io_factory_(new MockMessageIoFactory),
        mock_timeout_factory_(new MockTimeoutFactory),
        client_(mock_message_io_factory_, mock_timeout_factory_) {}

  void CreateMocks(int num_message_ios, int num_timeouts) {
    {
      InSequence seq;
      while (--num_message_ios >= 0) {
        mock_message_ios_.push_back(new MockMessageIo);
        EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
          .WillOnce(Return(mock_message_ios_.back()));
      }
    }

    {
      InSequence seq;
      while (--num_timeouts >= 0) {
        mock_timeouts_.push_back(new MockTimeout);
        EXPECT_CALL(*mock_timeout_factory_, MakeTimeout())
          .WillOnce(Return(mock_timeouts_.back()));
      }
    }
  }
  
  // The following are NOT OWNED
  MockMessageIoFactory *mock_message_io_factory_;
  MockTimeoutFactory *mock_timeout_factory_;
  // The pointees of the following vector members are NOT OWNED.
  vector<MockMessageIo *> mock_message_ios_;
  vector<MockTimeout *> mock_timeouts_;
  // The following ARE owned
  Client client_;
};

TEST_F(ClientTest, NormalRequestsSendOncePerCallback) {
  CreateMocks(1, 1);

  function<void(void)> callback;
  // Set expectations on other mocks
  EXPECT_CALL(*mock_message_ios_[0], SendTo(_, _))
    .Times(Exactly(kTrials))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(*mock_timeouts_[0], Start(_))
    .Times(Exactly(kTrials))
    .WillRepeatedly(SaveArg<0>(&callback));

  // Send request
  client_.SendGetTableRequest(kRouterId);

  // Start at 1 because it sends once immediately.
  for (int i = 1; i < kTrials; ++i)
    callback();
}

TEST_F(ClientTest, SendGetTableSendsWellFormedMessageExactlyOnce) {
  CreateMocks(1, 1);

  Message m(0, Message::UNKNOWN, 0);
  // Set expectations on other mocks
  EXPECT_CALL(*mock_message_ios_[0], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Return(true)));
  EXPECT_CALL(*mock_timeouts_[0], Start(_));

  // Send request.
  client_.SendGetTableRequest(kRouterId);

  // Check message
  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::REQUEST_TABLE);
  EXPECT_THAT(m.Table(), IsEmpty());
}

TEST_F(ClientTest, PushTableToSendsWellFormedMessageExactlyOnce) {
  CreateMocks(1, 1);

  Message m(0, Message::UNKNOWN, 0);
  // Set expectations on other mocks
  EXPECT_CALL(*mock_message_ios_[0], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Return(true)));
  EXPECT_CALL(*mock_timeouts_[0], Start(_));

  // Send request
  client_.PushTableTo(kRouterId, kTable);

  // Check Message
  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::PUSH_TABLE);
  EXPECT_EQ(m.Table(), kTable);
}

TEST_F(ClientTest, BroadcastTableSendsOneTableToEachNeighbor) {
  CreateMocks(kNeighbors.size(), kNeighbors.size());

  // Stores destination router ids.
  set<uint16_t> sent_to;
  for (unsigned i = 0; i < kNeighbors.size(); ++i) {
    EXPECT_CALL(*mock_message_ios_[i], SendTo(_, _))
      .WillOnce(DoAll(
            IgnoreResult(WithArg<1>(Invoke(&sent_to, 
                  static_cast<  // Cast picks which overload to use.
                      pair<set<uint16_t>::iterator, bool>   // return type.
                      (set<uint16_t>:: *)                   // member function.
                      (const uint16_t &)                    // arg type.
                    >(&set<uint16_t>::insert)))),
            Return(true)
            ));
    EXPECT_CALL(*mock_timeouts_[i], Start(_));
  }

  client_.BroadcastTable(kNeighbors, kTable);

  EXPECT_EQ(sent_to, kNeighbors);
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
    server_.reset(
        new Server(mock_message_io_factory_, mock_timeout_factory_, 1));

    server_->OnTableRequest([]() -> const map<uint16_t, int16_t> & {
        return kTable;
      });

    received_ = false;
    server_->OnTableReceipt([this](uint16_t source_id, 
          map<uint16_t, int16_t> table) {
        source_id_ = source_id;
        table_update_ = table;
        received_ = true;
      });
  }

  void StopServer() {
    server_.reset();

    mock_message_io_factory_ = new MockMessageIoFactory;
    mock_timeout_factory_ = new MockTimeoutFactory;
  }

  void CreateMocks(int num_message_ios, int num_timeouts) {
    {
      InSequence seq;
      while (--num_message_ios >= 0) {
        mock_message_ios_.push_back(new MockMessageIo);
        EXPECT_CALL(*mock_message_io_factory_, MakeMessageIo())
          .WillOnce(Return(mock_message_ios_.back()));
      }
    }

    {
      InSequence seq;
      while (--num_timeouts >= 0) {
        mock_timeouts_.push_back(new MockTimeout);
        EXPECT_CALL(*mock_timeout_factory_, MakeTimeout())
          .WillOnce(Return(mock_timeouts_.back()));
      }
    }
  }

  // The following are NOT OWNED
  MockMessageIoFactory *mock_message_io_factory_;
  MockTimeoutFactory *mock_timeout_factory_;
  // The pointees of the following vector members are NOT OWNED.
  vector<MockMessageIo *> mock_message_ios_;
  vector<MockTimeout *> mock_timeouts_;
  // The following ARE owned
  map<uint16_t, int16_t> table_update_;
  uint16_t source_id_;
  unique_ptr<Server> server_;
  atomic<bool> received_;
};

TEST_F(ServerTest, NormalResponseSendsEachTimeCallbackIsCalled) {
  CreateMocks(2, 1);

  function<void(void)> callback;
  EXPECT_CALL(*mock_message_ios_[0], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(0, Message::REQUEST_TABLE, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[1], SendTo(_, _))
    .Times(Exactly(kTrials))
    .WillRepeatedly(Return(true));
  EXPECT_CALL(*mock_timeouts_[0], Start(_))
    .Times(Exactly(kTrials))
    .WillRepeatedly(SaveArg<0>(&callback));

  StartServer();
  while (!callback)
    this_thread::yield();  // Wait unti callback is set

  for (int i = 1; i < kTrials; ++i)
    callback();
}

TEST_F(ServerTest, AckIsSentOnlyOnce) {
  CreateMocks(2, 0);

  atomic<bool> wait(true);
  EXPECT_CALL(*mock_message_ios_[0], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(kDestId, Message::TABLE_RESPONSE, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[1], SendTo(_, _))
    .WillOnce(DoAll(
          Assign(&wait, false),
          Return(true)
          ));
  
  EXPECT_CALL(*mock_timeout_factory_, MakeTimeout()).Times(Exactly(0));

  StartServer();
  while (wait)
    this_thread::yield();
}

TEST_F(ServerTest, SendsWellFormedTableResponseOnRequestTable) {
  CreateMocks(2, 1);

  atomic<bool> wait(true);
  Message m(0, Message::UNKNOWN, 0);
  EXPECT_CALL(*mock_message_ios_[0], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(0, Message::REQUEST_TABLE, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[1], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Assign(&wait, false),
          Return(true)
          ));
  EXPECT_CALL(*mock_timeouts_[0], Start(_));

  StartServer();
  while (wait)
    this_thread::yield();

  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::TABLE_RESPONSE);
  EXPECT_EQ(m.DestinationId(), kSourceId);
  EXPECT_EQ(m.Table(), kTable);
}

TEST_F(ServerTest, SendsWellFormedAckOnTableResponse) {
  CreateMocks(2, 0);
  
  atomic<bool> wait(true);
  Message m(0, Message::UNKNOWN, 0);
  EXPECT_CALL(*mock_message_ios_[0], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(kDestId, Message::TABLE_RESPONSE, kSourceId, kTable))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[1], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Assign(&wait, false),
          Return(true)
          ));

  StartServer();
  while (wait)
    this_thread::yield();

  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::ACK);
  EXPECT_EQ(m.DestinationId(), kSourceId);
  EXPECT_EQ(m.SourceId(), kDestId);
  EXPECT_THAT(m.Table(), IsEmpty());

  // On Table Receipt result.
  while (!received_)
    this_thread::yield();

  EXPECT_EQ(table_update_, kTable);
}

TEST_F(ServerTest, SendsWellFormedAckOnPushTableReceipt) {
  CreateMocks(2, 0);
  
  atomic<bool> wait(true);
  Message m(0, Message::UNKNOWN, 0);
  EXPECT_CALL(*mock_message_ios_[0], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(Message(0, Message::PUSH_TABLE, kSourceId, kTable))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[1], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Assign(&wait, false),
          Return(true)
          ));

  StartServer();
  while (wait)
    this_thread::yield();

  EXPECT_TRUE(m.IsValid());
  EXPECT_EQ(m.GetType(), Message::ACK);
  EXPECT_EQ(m.DestinationId(), kSourceId);
  EXPECT_THAT(m.Table(), IsEmpty());
  
  // On Table Receipt result.
  while (!received_)
    this_thread::yield();

  EXPECT_EQ(table_update_, kTable);
}

TEST_F(ServerTest, SendsNothingWhenAckIsReceived) {
  CreateMocks(1, 0);

  atomic<bool> wait(true);
  EXPECT_CALL(*mock_message_ios_[0], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Assign(&wait, false),
          Return(Message(kDestId, Message::ACK, kSourceId))
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));

  StartServer();
  while (wait)
    this_thread::yield();
}

// Tests complete flow paths by acting as both client and server.
class FlowTest : public ::testing::Test {
 protected:
  static void TearDownTestCase() {
    ResetAll();
  }

  FlowTest()
      : client_mock_message_io_factory_(new MockMessageIoFactory),
        client_mock_timeout_factory_(new MockTimeoutFactory),
        server_mock_message_io_factory_(new MockMessageIoFactory),
        server_mock_timeout_factory_(new MockTimeoutFactory),
        client_(client_mock_message_io_factory_, client_mock_timeout_factory_)
  {}

  void StartServer() {
    server_.reset(new Server(server_mock_message_io_factory_,
          server_mock_timeout_factory_, 1));

    server_->OnTableRequest([]() -> const map<uint16_t, int16_t> & {
        return kTable;
      });

    server_->OnTableReceipt([this](uint16_t source_id, 
          map<uint16_t, int16_t> table) {
        source_id_ = source_id;
        table_update_ = table;
      });
  }

  void StopServer() {
    server_.reset();

    server_mock_message_io_factory_ = new MockMessageIoFactory;
    server_mock_timeout_factory_ = new MockTimeoutFactory;
  }

  void CreateMocks(int num_message_ios, int num_timeouts, bool client) {
    MockMessageIoFactory *mock_message_io_factory = (client 
        ? client_mock_message_io_factory_ 
        : server_mock_message_io_factory_);
    MockTimeoutFactory *mock_timeout_factory = (client
        ? client_mock_timeout_factory_ 
        : server_mock_timeout_factory_);

    {
      InSequence seq;
      while (--num_message_ios >= 0) {
        mock_message_ios_.push_back(new MockMessageIo);
        EXPECT_CALL(*mock_message_io_factory, MakeMessageIo())
          .WillOnce(Return(mock_message_ios_.back()));
      }
    }

    {
      InSequence seq;
      while (--num_timeouts >= 0) {
        mock_timeouts_.push_back(new MockTimeout);
        EXPECT_CALL(*mock_timeout_factory, MakeTimeout())
          .WillOnce(Return(mock_timeouts_.back()));
      }
    }
  }

  // The following are NOT OWNED
  MockMessageIoFactory *client_mock_message_io_factory_;
  MockTimeoutFactory *client_mock_timeout_factory_;
  MockMessageIoFactory *server_mock_message_io_factory_;
  MockTimeoutFactory *server_mock_timeout_factory_;
  // The pointees of the following vector members are NOT OWNED.
  vector<MockMessageIo *> mock_message_ios_;
  vector<MockTimeout *> mock_timeouts_;
  // The following ARE owned
  map<uint16_t, int16_t> table_update_;
  uint16_t source_id_;
  unique_ptr<Server> server_;
  Client client_;
};

// Executes the entire request flow as both the client and server.
TEST_F(FlowTest, RequestFlow) {
  CreateMocks(1, 1, true);  // Create Mocks for the client.
  
  Message m(0, Message::UNKNOWN, 0);
  EXPECT_CALL(*mock_message_ios_[0], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Return(true)
          ));
  EXPECT_CALL(*mock_timeouts_[0], Start(_));

  // Send REQUEST_TABLE
  client_.SendGetTableRequest(kRouterId);

  CreateMocks(2, 1, false);  // Create Mocks for the server.

  atomic<bool> wait(true);
  EXPECT_CALL(*mock_message_ios_[1], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kLocalId),
          SetArgReferee<1>(true),
          Return(m)
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[2], SendTo(_, kLocalId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Assign(&wait, false),
          Return(true)
          ));
  EXPECT_CALL(*mock_timeouts_[1], Start(_));

  // Process REQUEST_TABLE -> Send TABLE_RESPONSE
  StartServer();
  while (wait)
    this_thread::yield();
  StopServer();

  CreateMocks(2, 0, false);  // Create Mocks for the server.
  
  wait = true;
  EXPECT_CALL(*mock_message_ios_[3], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Return(m)
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[4], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Assign(&wait, false),
          Return(true)
          ));
  // Expect that receipt of response deactivates request timeout.
  EXPECT_CALL(*mock_timeouts_[0], Cancel());
  
  // Process TABLE_RESPONSE -> Send ACK
  StartServer();
  while (wait)
    this_thread::yield();
  StopServer();

  CreateMocks(1, 0, false);  // Create Mocks for the server.

  wait = true;
  EXPECT_CALL(*mock_message_ios_[5], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kLocalId),
          SetArgReferee<1>(true),
          Assign(&wait, false),
          Return(m)
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  // Expect that receipt of ACK deactivates response timeout.
  EXPECT_CALL(*mock_timeouts_[1], Cancel());
  
  // Process ACK
  StartServer();
  while (wait)
    this_thread::yield();
}

// Executes the entire async push flow as both the client and server.
TEST_F(FlowTest, AsyncPushFlow) {
  CreateMocks(1, 1, true);  // Create Mocks for the client.
  
  Message m(0, Message::UNKNOWN, 0);
  EXPECT_CALL(*mock_message_ios_[0], SendTo(_, kRouterId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Return(true)
          ));
  EXPECT_CALL(*mock_timeouts_[0], Start(_));

  // Send PUSH_TABLE
  client_.PushTableTo(kRouterId, kTable);

  CreateMocks(2, 0, false);  // Create Mocks for the server.
  
  atomic<bool> wait(true);
  EXPECT_CALL(*mock_message_ios_[1], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kLocalId),
          SetArgReferee<1>(true),
          Return(m)
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  EXPECT_CALL(*mock_message_ios_[2], SendTo(_, kLocalId))
    .WillOnce(DoAll(
          SaveArg<0>(&m),
          Assign(&wait, false),
          Return(true)
          ));
  
  // Process PUSH_TABLE -> Send ACK
  StartServer();
  while (wait)
    this_thread::yield();
  StopServer();

  CreateMocks(1, 0, false);  // Create Mocks for the server.

  wait = true;
  EXPECT_CALL(*mock_message_ios_[3], ReceiveFrom(_, _))
    .WillOnce(DoAll(
          SetArgReferee<0>(kRouterId),
          SetArgReferee<1>(true),
          Assign(&wait, false),
          Return(m)
          ))
    .WillRepeatedly(DoAll(
          SetArgReferee<1>(false),
          Return(Message(0, Message::UNKNOWN, 0))
          ));
  // Expect that receipt of ACK deactivates push timeout.
  EXPECT_CALL(*mock_timeouts_[0], Cancel());
  
  // Process ACK
  StartServer();
  while (wait)
    this_thread::yield();
}
}  // anonymous namespace
}  // namespace router
