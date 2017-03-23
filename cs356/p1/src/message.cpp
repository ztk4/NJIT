#include "message.h"

using namespace std;

namespace router {
Message::Message(uint16_t dest_id, Type type, uint16_t src_id,
    const map<uint16_t, int16_t> &table)
    : dest_id_(dest_id),
      type_(type),
      src_id_(src_id),
      table_(table) {
  if (src_id == 0)
    type_ = UNKNOWN;
}

bool Message::operator==(const Message &m) const {
  return (
      dest_id_ == m.dest_id_ &&
      type_ == m.type_ &&
      src_id_ == m.src_id_ &&
      table_ == m.table_);
}

bool Message::operator!=(const Message &m) const {
  return !(*this == m);
}

ssize_t Message::Serialize(void *buf, size_t len) const {
  size_t size = 8 + 4 * table_.size();
  if (size > len)
    return -1;

  // Segment into unsigned 2-byte words.
  uint16_t *buf16 = reinterpret_cast<uint16_t *>(buf);
  *buf16++ = dest_id_;                // Destination Message ID.
  *buf16++ = ((type_ & 0x3F) << 2);   // Opcode shifted 2 bits up in LSB.
  *buf16++ = src_id_;                 // Source Message ID.
  *buf16++ = table_.size();           // Number of table rows.
  
  for (const auto &entry : table_) {
    *buf16++ = entry.first;                                         // Router ID
    *buf16++ = *reinterpret_cast<const uint16_t *>(&entry.second);  // Distance
  }

  return size;
}

Message Message::Deserialize(const void *buf, size_t len) {
  // Must have at least 8 bytes for header
  if (len < 8)
    return Message(0, UNKNOWN, 0);

  // Segment into unsigned 2-byte words.
  const uint16_t *buf16 = reinterpret_cast<const uint16_t *>(buf);
  size_t num_rows = buf16[3];
  uint16_t tmp_type = buf16[1] >> 2;

  // If len is not the expected size or bad source id or bad type,
  // return an error Message.
  if (len != 8 + num_rows * 4 || buf16[2] == 0 || tmp_type >= UNKNOWN)
    return Message(0, UNKNOWN, 0);

  map<uint16_t, int16_t> table;
  for (const uint16_t *buf16_p = buf16 + 4;
      buf16_p < buf16 + len/2;
      buf16_p += 2) {
    table[*buf16_p] = *reinterpret_cast<const int16_t *>(buf16_p + 1);
  }

  return Message(buf16[0], (Type) tmp_type, buf16[2], table);
}
}  // namespace router
