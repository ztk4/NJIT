#include "message.h"

using namespace std;

Message::Message(int16_t id, Type type, const map<int16_t, int16_t> &table)
    : id_(id),
      type_(type),
      table_(table) {}

ssize_t Message::Serialize(void *buf, size_t len) const {
  size_t size = 4 + 4 * table_.size();
  if (size > len)
    return -1;

  // Segment into 2-byte words.
  int16_t *buf16 = (int16_t *) buf;
  *buf16++ = id_;             // Message ID.
  *buf16++ = (type_ << 2);    // Opcode shifted 2 bits up in LSB.
  
  for (const auto &entry : table_) {
    *buf16++ = entry.first;   // Router ID.
    *buf16++ = entry.second;  // Distance
  }

  return size;
}

Message Message::Deserialize(void *buf, size_t len) {
  // Segment into 2-byte words.
  int16_t *buf16 = (int16_t *) buf;

  map<int16_t, int16_t> table;
  for (int16_t *buf16_p = buf16 + 2; buf16_p < buf16 + len/2; buf16_p += 2) {
    table[*buf16_p] = buf16_p[1];
  }

  int16_t tmp_type = buf16[1] >> 2;
  Type type = (tmp_type >= UNKNOWN ? UNKNOWN : (Type) tmp_type);

  return Message(*buf16, type, table);
}
