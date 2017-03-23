#ifndef ROUTER_MESSAGE_IO_H_
#define ROUTER_MESSAGE_IO_H_

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "message.h"
#include "socket.h"

/// Namespace for all components directly related to the routing protocol.
namespace router {
/// An interface for sending and receiving messages.
class MessageIo {
 public:
  virtual ~MessageIo() = default;

  /// Sends the specified message to the specified router.
  ///
  /// @param m the message to send.
  /// @param router_id the identifier of the router to send to.
  ///
  /// @returns true if the message was sent successfully, false otherwise.
  virtual bool SendTo(const Message &m, uint16_t router_id) = 0;

  /// Recieves a message from any address. This method blocks on receving a
  /// packet, and returns on the first packet received.
  ///
  /// @param router_id this reference is set to the identifier of the router
  ///                  the message is received from.
  /// @param status this is set true on success, and false on failure.
  ///
  /// @returns the message received.
  virtual Message ReceiveFrom(uint16_t &rotuer_id, bool &status) = 0;

 protected:
  MessageIo() = default;
};
/// Interface for creating MessageIo's.
class MessageIoFactory {
 public:
  virtual ~MessageIoFactory() = default;

  /// @returns a pointer to a new MessageIo. The caller takes ownership of the
  ///          pointer.
  virtual MessageIo *MakeMessageIo() = 0;

 protected:
  MessageIoFactory() = default;
};

/// Implementation of MessageIo using Sockets.
class SocketMessageIo : public MessageIo {
 public:
  ~SocketMessageIo() override = default;

  // MUTEX LOCKS EXCLUDED
  bool SendTo(const Message &m, uint16_t router_id) override;
  // MUTEX LOCKS EXCLUDED
  Message ReceiveFrom(uint16_t &router_id, bool &status) override;

  /// Adds an address and router pair to both the forward and reverse lookup
  /// tables.
  /// MUTEX LOCKS EXCLUDED.
  ///
  /// @param router_id the router id in the pair.
  /// @param addr the IPv4 address in the pair.
  static void AddRouterAddressPair(uint16_t router_id,
      const util::InSocket::InAddress &addr);

  /// Injects the socket for use by this class. Typically bound to a predictable
  /// port for recv calls. Takes ownership of the socket.
  ///
  /// @param the socket. Takes ownership of this socket.
  static void SetSocket(util::Socket *s);

  /// Closes the internal socket used by this class.
  static void CloseSocket();

 private:
  static std::unordered_map<uint16_t, util::InSocket::InAddress>
    addr_lookup_table_;
  static std::unordered_map<util::InSocket::InAddress, uint16_t>
    rev_lookup_table_;
  static std::mutex addr_lookup_table_mutex_;   /// GUARDS addr_lookup_table_.
  static std::mutex rev_lookup_table_mutex_;    /// GUARDS rev_lookup_table_.
  static std::unique_ptr<util::Socket> socket_;
};
/// Implementation of MessageIoFactory for SocketMessageIo's
class SocketMessageIoFactory : public MessageIoFactory {
 public:
  ~SocketMessageIoFactory() override = default;

  MessageIo *MakeMessageIo() override { return new SocketMessageIo; }
};
}  // namespace router

#endif  // ROUTER_MESSAGE_IO_H_
