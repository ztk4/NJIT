#ifndef UTIL_SOCKET_H_
#define UTIL_SOCKET_H_

#include <functional>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

/// Namespace used for standard utilities.
namespace util {
/// An interface for wrapping socket.
/// Only exposing relevant socket methods for simplicity.
class Socket {
 public:
  /// An interface for wrapping sockaddr with, allows for inheritance of address
  /// types.
  class Address {
   public:
    virtual ~Address() = default;

    /// @returns a pointer to the sockaddr represented by this Address.
    virtual struct sockaddr *Value() = 0;
    /// @returns a const pointer to the sockaddr represented by this Address.
    virtual const struct sockaddr *Value() const = 0;

    /// @returns the length of the sockaddr represented by this Address.
    virtual socklen_t Length() const = 0;

   protected:
    Address() = default;
  };
  virtual ~Socket() = default;

  /// Binds this Socket to an address.
  ///
  /// @param addr a reference to the binding address.
  ///
  /// @returns true if the bind succeeds, false otherwise.
  virtual bool Bind(const Address *addr) = 0;

  /// Recieves a packet on this socket, passing back the body of the packet,
  /// and the address the packet was recieved from.
  ///
  /// @param buffer the buffer to write the body of the packet to.
  /// @param length the maximum number of bytes to write to buffer.
  /// @param flags recvfrom flags as specified by <sys/socket.h>.
  /// @param addr will be set to the source address of the packet.
  ///
  /// @returns the length of the packet written to buffer, 0 if no message is
  ///          received, or -1 on error.
  virtual ssize_t RecvFrom(void *buffer, size_t length, int flags,
      Address *addr) = 0;

  /// Sends a packet on this socket to the specified address.
  ///
  /// @param message a void pointer to the message to send as the packet.
  /// @param length the length of the message to send.
  /// @param flags sendto flags as specified by <sys/socket.h>.
  /// @param addr the address to send the message to.
  ///
  /// @rerturns the number of bytes sent, or -1 upon error.
  virtual ssize_t SendTo(const void *message, size_t length, int flags,
      const Address *addr) = 0;

  /// @returns true if this socket is open, false otherwise
  virtual bool IsOpen() const = 0;

 protected:
  Socket() = default;
};

/// Socket in the Internet Domain using IPv4
class InSocket : public Socket {
 public:
  /// Internet Socket (Only supports UDP for now)
  enum Type {
    UDP,
  };

  /// Impementation of Socket::Address in the Internet Address Family
  class InAddress : public Address {
   public:
    /// Construct an internet address.
    ///
    /// @param addr the IPv4 address.
    /// @param port the port on that address.
    explicit InAddress(in_addr_t addr = 0, in_port_t port = 0);
    /// Copy Constructor
    InAddress(const InAddress &addr);
    /// Copy Assignment Operator
    InAddress &operator=(const InAddress &addr);
    ~InAddress() override = default;

    struct sockaddr *Value() override;
    const struct sockaddr *Value() const override;
    socklen_t Length() const override;

    // Comparison Operators (needed for reverse address lookup).
    bool operator==(const InAddress &addr) const;
    bool operator!=(const InAddress &addr) const;

    /// @returns a 48-bit hash of an InAddress (needed for reverse addr lookup).
    size_t Hash() const;

   private:
    struct sockaddr_in addr_;  /// Internet address struct.
  };

  /// Constructs a socket in the internet domain of the specified type.
  ///
  /// @param type the type of internet socket to create.
  explicit InSocket(Type type);
  // Disallow Copy and Assign
  InSocket(const InSocket &) = delete;
  InSocket &operator=(const InSocket &) = delete;
  ~InSocket() override;
  
  bool Bind(const Address *addr) override;
  ssize_t RecvFrom(void *buffer, size_t length, int flags,
      Address *addr) override;
  ssize_t SendTo(const void *message, size_t length, int flags,
      const Address *addr) override;

  bool IsOpen() const override { return socket_ >= 0; }

 private:
  int socket_ = -1;  /// Socket file descriptor.
};
}  // namespace util

// LOGGING
std::ostream &operator<<(std::ostream &o,
    const util::InSocket::InAddress &addr);

namespace std {
/// Sepcialization of std::hash to allow hashing of InAddress
template<>
struct hash<util::InSocket::InAddress> {
  size_t operator()(const util::InSocket::InAddress &addr) const {
    return addr.Hash();
  }
};
}  // namespace std

#endif  // UTIL_SOCKET_H_
