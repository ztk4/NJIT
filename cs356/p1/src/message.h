#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <cstdint>
#include <map>

/// Object representation of any message sent via the routing protocol. Can be
/// thought of as a de-serialized version of a message.
class Message {
 public:
  /// This enum is equivalent to the Opcode field in the protocol
  enum Type : char {
    ACK,
    REQUEST_TABLE,
    TABLE_RESPONSE,
    PUSH_TABLE,
    UNKNOWN,
  };

  /// Creates a Message object from a unique identifier, a type/opcode, and a
  /// routing table mapping a 16-bit router identifier to distances.
  ///
  /// @param id 16-bit identifier for this message.
  /// @param type the type of this message, based off of section II. in the
  ///        router protocol.
  /// @param table a routers current table, mapping 16-bit router
  ///        identifiers to signed 16-bit distances. A -1 means an unknown
  ///        distance.
  Message(int16_t id, Type type,
      const std::map<int16_t, int16_t> &table = std::map<int16_t, int16_t>());

  /// @returns the Id of this message.
  int16_t Id() const { return id_; }

  /// @returns the type of this message.
  Type GetType() const { return type_; }

  /// @returns a const reference to the table from the body of this message.
  const std::map<int16_t, int16_t> &Table() const { return table_; }

  /// Serializes the message object into a message that can be sent over UDP.
  ///
  /// @param buf the output buffer.
  /// @param len the max size in bytes of the output buffer.
  ///
  /// @return the total number of bytes written to buf, or -1 if there was not
  ///         enough space in buf.
  ssize_t Serialize(void *buf, size_t len) const;

  /// Deserializes a message that was sent over UDP into a Message object.
  ///
  /// @param buf the input buffer.
  /// @param len the size in bytes of the input buffer.
  ///
  /// @returns a deserialized Message object.
  static Message Deserialize(void *buf, size_t len);

 private:
  int16_t id_;
  Type type_;
  std::map<int16_t, int16_t> table_;
};

#endif  // MESSAGE_H_
