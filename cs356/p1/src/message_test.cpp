#include "message.h"

#include <cstring>

#include "gtest/gtest.h"

using namespace std;

namespace {
const int kId = 42;
const map<int16_t, int16_t> kTable = {
  {0, 0}, {15, -1}, {255, 63}, {1023, 31}
};

TEST(MessageTest, ConstructorInitializesAllFields) {
  Message m(kId, Message::PUSH_TABLE, kTable);

  EXPECT_EQ(m.Id(), kId);
  EXPECT_EQ(m.GetType(), Message::PUSH_TABLE);
  EXPECT_EQ(m.Table(), kTable);
}

TEST(MessageTest, ConstructorDefaultInitializesTable) {
  Message m(kId, Message::ACK);

  EXPECT_EQ(m.Id(), kId);
  EXPECT_EQ(m.GetType(), Message::ACK);
  EXPECT_TRUE(m.Table().empty());
}

TEST(MessageTest, SerializeProducesExpectedMessageWithoutTable) {
  Message m(kId, Message::REQUEST_TABLE);

  char buf[10];
  ssize_t size = m.Serialize(buf, 10);

  EXPECT_EQ(size, 4);  // Should just contain header

  // The expected serialization
  char expected[] = {0x2A, 0x00, 0x04, 0x00};

  EXPECT_EQ(memcmp(buf, expected, 4), 0);
}

TEST(MessageTest, SerializeProducesExpectedMessageWithTable) {
  Message m(kId, Message::TABLE_RESPONSE, kTable);

  char buf[32];
  ssize_t size = m.Serialize(buf, 32);

  EXPECT_EQ(size, 20);  // Should be header plus 4 table rows

  char expected_hdr[] = {0x2A, 0x00, 0x08, 0x00};

  EXPECT_EQ(memcmp(buf, expected_hdr, 4), 0);

  // Can't memcmp because order of iterating map is not something to rely on.
  for (int i = 4; i < 20; i += 4) {
    int16_t *row = (int16_t *) (buf + i);
    EXPECT_EQ(kTable.at(*row), row[1]);
  }
}

TEST(MessageTest, SerializeReturnsErrorWhenBufferIsNotLargeEnough) {
  Message m(kId, Message::PUSH_TABLE, kTable);

  char buf[16];
  ssize_t size = m.Serialize(buf, 16);

  EXPECT_EQ(size, -1);
}

TEST(MessageTest, DeserializePopulatesMessageCorrectlyWithoutTable) {
  uint8_t serialized[] = {0x2A, 0x0F, 0x00, 0x00};

  Message m = Message::Deserialize(serialized, 4);

  EXPECT_EQ(m.Id(), 0xF2A);
  EXPECT_EQ(m.GetType(), Message::ACK);
  EXPECT_TRUE(m.Table().empty());
}

TEST(MessageTest, DeserializePopulatesMessageCorrectlyWithTable) {
  uint8_t serialized[] = {
    0x2A, 0xFF, 0x08, 0x00,
    0xAD, 0xDE, 0xEF, 0xBE,
    0xFF, 0x00, 0x00, 0xFF,
    0xCD, 0xAB, 0x00, 0xEF,
  };

  Message m = Message::Deserialize(serialized, 16);

  uint16_t id = 0xFF2A;

  EXPECT_EQ(m.Id(), *(int16_t *)&id);
  EXPECT_EQ(m.GetType(), Message::TABLE_RESPONSE);
  EXPECT_EQ(m.Table().size(), 3);  // 3 rows

  uint16_t first = 0xBEEF,
           second = 0xFF00,
           third = 0xEF00;

  // Each Row
  EXPECT_EQ(m.Table().at(0xDEAD), *(int16_t *)&first);
  EXPECT_EQ(m.Table().at(0x00FF), *(int16_t *)&second);
  EXPECT_EQ(m.Table().at(0xABCD), *(int16_t *)&third);
}

TEST(MessageTest, DeserializePopulatesMessageWithErrorWhenOpcodeIsUnknown) {
  uint8_t serialized[] = {0x2A, 0x0F, 0xFF, 0x2A};

  Message m = Message::Deserialize(serialized, 4);

  EXPECT_EQ(m.Id(), 0xF2A);
  EXPECT_EQ(m.GetType(), Message::UNKNOWN);
  EXPECT_TRUE(m.Table().empty());
}
}  // anonymous namespace
