#include "message.h"

#include <cstring>

#include "gtest/gtest.h"

using namespace std;

namespace {
const int kDestinationId = 42, kSourceId = 17;
const map<uint16_t, int16_t> kTable = {
  {0, 0}, {15, -1}, {255, 63}, {0xFFFF, 0x7FFF}
};

TEST(MessageTest, ConstructorInitializesAllFields) {
  Message m(kDestinationId, Message::PUSH_TABLE, kSourceId, kTable);

  EXPECT_EQ(m.DestinationId(), kDestinationId);
  EXPECT_EQ(m.GetType(), Message::PUSH_TABLE);
  EXPECT_EQ(m.SourceId(), kSourceId);
  EXPECT_EQ(m.Table(), kTable);
  EXPECT_TRUE(m.IsValid());
}

TEST(MessageTest, ConstructorDefaultInitializesTable) {
  Message m(kDestinationId, Message::ACK, kSourceId);

  EXPECT_EQ(m.DestinationId(), kDestinationId);
  EXPECT_EQ(m.GetType(), Message::ACK);
  EXPECT_EQ(m.SourceId(), kSourceId);
  EXPECT_TRUE(m.Table().empty());
  EXPECT_TRUE(m.IsValid());
}

TEST(MessageTest, MessageWithUnknownTypeIsInvalid) {
  Message m(kDestinationId, Message::UNKNOWN, kSourceId);

  EXPECT_FALSE(m.IsValid());
}

TEST(MessageTest, MessageWithZeroSourceIdIsInvalid) {
  Message m(kDestinationId, Message::ACK, 0);

  EXPECT_FALSE(m.IsValid());
}

TEST(MessageTest, SerializeProducesExpectedMessageWithoutTable) {
  Message m(kDestinationId, Message::REQUEST_TABLE, kSourceId);

  char buf[10];
  ssize_t size = m.Serialize(buf, 10);

  EXPECT_EQ(size, 8);  // Should just contain header

  // The expected serialization
  char expected[] = {
    0x2A, 0x00, 0x04, 0x00,
    0x11, 0x00, 0x00, 0x00,
  };

  EXPECT_EQ(memcmp(buf, expected, 8), 0);
}

TEST(MessageTest, SerializeProducesExpectedMessageWithTable) {
  Message m(kDestinationId, Message::TABLE_RESPONSE, kSourceId, kTable);

  char buf[32];
  ssize_t size = m.Serialize(buf, 32);

  EXPECT_EQ(size, 24);  // Should be header plus 4 table rows

  char expected_hdr[] = {
    0x2A, 0x00, 0x08, 0x00,
    0x11, 0x00, 0x04, 0x00,
  };

  EXPECT_EQ(memcmp(buf, expected_hdr, 8), 0);

  // Can't memcmp because order of iterating map is not something to rely on.
  for (int i = 8; i < 24; i += 4) {
    uint16_t *row = reinterpret_cast<uint16_t *>(buf + i);
    auto it = kTable.find(*row);
    EXPECT_FALSE(it == kTable.end());
    EXPECT_EQ(it->second, *reinterpret_cast<int16_t *>(row + 1));
  }
}

TEST(MessageTest, SerializeReturnsErrorWhenBufferIsNotLargeEnough) {
  Message m(kDestinationId, Message::PUSH_TABLE, kSourceId, kTable);

  char buf[16];
  ssize_t size = m.Serialize(buf, 16);

  EXPECT_EQ(size, -1);
}

TEST(MessageTest, DeserializePopulatesMessageCorrectlyWithoutTable) {
  uint8_t serialized[] = {
    0x2A, 0xFF, 0x00, 0x00,
    0x11, 0xFF, 0x00, 0x00,
  };

  Message m = Message::Deserialize(serialized, 8);

  EXPECT_EQ(m.DestinationId(), 0xFF2A);
  EXPECT_EQ(m.GetType(), Message::ACK);
  EXPECT_EQ(m.SourceId(), 0xFF11);
  EXPECT_TRUE(m.Table().empty());
  EXPECT_TRUE(m.IsValid());
}

TEST(MessageTest, DeserializePopulatesMessageCorrectlyWithTable) {
  uint8_t serialized[] = {
    0x2A, 0xFF, 0x08, 0x00,
    0x11, 0xFF, 0x03, 0x00,
    0xAD, 0xDE, 0xEF, 0xBE,
    0xFF, 0x00, 0x00, 0xFF,
    0xCD, 0xAB, 0x00, 0xEF,
  };

  Message m = Message::Deserialize(serialized, 20);

  EXPECT_EQ(m.DestinationId(), 0xFF2A);
  EXPECT_EQ(m.GetType(), Message::TABLE_RESPONSE);
  EXPECT_EQ(m.SourceId(), 0xFF11);
  EXPECT_EQ(m.Table().size(), 3);  // 3 rows

  uint16_t first = 0xBEEF,
           second = 0xFF00,
           third = 0xEF00;

  // Each Row
  EXPECT_EQ(m.Table().at(0xDEAD), *reinterpret_cast<int16_t *>(&first));
  EXPECT_EQ(m.Table().at(0x00FF), *reinterpret_cast<int16_t *>(&second));
  EXPECT_EQ(m.Table().at(0xABCD), *reinterpret_cast<int16_t *>(&third));
}

TEST(MessageTest, DeserializePopulatesMessageWithErrorWhenOpcodeIsUnknown) {
  uint8_t serialized[] = {
    0x2A, 0x0F, 0xFF, 0x2A,
    0x11, 0x0A, 0x00, 0x00,
  };

  Message m = Message::Deserialize(serialized, 8);

  EXPECT_FALSE(m.IsValid());
}

TEST(MessageTest, DeserializePopulatesMessageWithErrorWhenLengthIsTooSmall) {
  uint8_t serialized[] = {0xDE, 0xAD, 0xBE, 0xEF};
  Message m = Message::Deserialize(serialized, 4);

  EXPECT_FALSE(m.IsValid());
}

TEST(MessageTest, DeserializePopulatesMessageWithErrorWhenLengthDoesNotMatch) {
  uint8_t serialized[] = {
    0x2A, 0x0F, 0x00, 0x00,
    0x11, 0xAA, 0x07, 0x00,
    0xDE, 0xAD, 0xBE, 0xEF,
  };

  Message m = Message::Deserialize(serialized, 12);

  EXPECT_FALSE(m.IsValid());
}

TEST(MessageTest, DeserializePopulatesMessageWithErrorWhenSourceIdIsZero) {
  uint8_t serialized[] = {
    0x2A, 0x0F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };

  Message m = Message::Deserialize(serialized, 8);

  EXPECT_FALSE(m.IsValid());
}
}  // anonymous namespace
