#ifndef ROUTER_MESSAGE_H_
#define ROUTER_MESSAGE_H_

#include <cstdint>
#include <map>

/// Namespace for all classes directly related to the routing protocol.
namespace router {
/// Object representation of any message sent via the routing protocol. Can be
/// thought of as a de-serialized version of a message.
class Message {
 public:
  /// This is the version number of the protocol implemented by this class.
  static const uint8_t kMajorVersion;  // Maximum value of 31
  static const uint8_t kMinorVersion;  // Maximum value of 15
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
  /// @param dest_id destination unsigned 16-bit identifier for this message.
  /// @param type the type of this message, based off of section II. in the
  ///        router protocol.
  /// @param src_id source unsigned 16-bit identifier for this message.
  /// @param table a routers current table, mapping unsigned 16-bit router
  ///        identifiers to signed 16-bit distances. A -1 means an unknown
  ///        distance.
  Message(uint16_t dest_id, Type type, uint16_t src_id,
      const std::map<uint16_t, int16_t> &table = std::map<uint16_t, int16_t>());

  /// @returns true if this message is valid.
  bool IsValid() const { return type_ != UNKNOWN; }

  /// Comparison Operators
  bool operator==(const Message &m) const;
  bool operator!=(const Message &m) const;

  /// @returns the Destination Id of this message.
  uint16_t DestinationId() const { return dest_id_; }
  /// @returns the Source Id of this message.
  uint16_t SourceId() const { return src_id_; }

  /// @returns the type of this message.
  Type GetType() const { return type_; }

  const char *GetTypeString() const;

  /// @returns a const reference to the table from the body of this message.
  const std::map<uint16_t, int16_t> &Table() const { return table_; }

  /// @returns the serialized size of the message in bytes.
  size_t SerializedLength() const { return 8 + 4 * table_.size(); }

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
  static Message Deserialize(const void *buf, size_t len);

 private:
  uint16_t dest_id_;
  Type type_;
  uint16_t src_id_;
  std::map<uint16_t, int16_t> table_;
};
}  // namespace router

#endif  // ROUTER_MESSAGE_H_
