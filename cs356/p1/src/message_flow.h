#ifndef ROUTER_MESSAGE_FLOW_H_
#define ROUTER_MESSAGE_FLOW_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <map>

#include "message_io.h"
#include "timeout.h"

/// Namespace for components directly related to the routing protocol.
namespace router {
/// Resets all ongoing connections. Removes all expectations of responses.
void ResetAll();
/// Client Class. Contains entry points for each type of message flow, and does
/// not do any threading.
class Client {
 public:
  /// Creates a client using a MessageIoFactory and a TimeoutFactory to start
  /// and manage flows. Takes ownership of both factories.
  ///
  /// @param message_io_factory the factory to make message_io objects from.
  /// @param timeout_factory the factory to make timeouts from.
  Client(MessageIoFactory *message_io_factory,
      util::TimeoutFactory *timeout_factory);

  /// Sends a Get Table Request message to the router specified.
  ///
  /// @param router_id the router to send the request to.
  /// 
  /// @returns true on success, false on failure.
  void SendGetTableRequest(uint16_t router_id);

  /// Sends a Push Table message containing the specified table to the router
  /// specified.
  ///
  /// @param router_id the router to send the request to.
  /// @param table a const reference to this router's routing table.
  ///
  /// @returns true on success, false on failure.
  void PushTableTo(uint16_t router_id,
      const std::map<uint16_t, int16_t> &table);

 private:
  std::unique_ptr<MessageIoFactory> message_io_factory_;
  std::unique_ptr<util::TimeoutFactory> timeout_factory_;
};

/// Server Class. On construction, spawns a thread to manage the server object,
/// and cleans up on destruction. Each server object independently listens for
/// requests and serves them as the come in.
class Server {
 public:
  /// Creates a server using a MessageIoFactory and a TimeoutFactory to 
  /// manage flows. Takes ownership of both factories.
  ///
  /// @param message_io_factory the factory to make message_io objects from.
  /// @param timeout_factory the factory to make timeouts from.
  Server(MessageIoFactory *message_io_factory,
      util::TimeoutFactory *timeout_factory);
  ~Server();

  /// Attaches a callback to the receipt of table event. Attaching a new
  /// callback will overwrite a previously attached callback.
  ///
  /// @param callback the function that will be called with the received table.
  void OnTableReceipt(
      const std::function<void(const std::map<uint16_t, int16_t> &)> &callback);

  /// Attaches a callback to the request for this routers table. Attaching a
  /// new callback will overwrite a previously attacked callback.
  ///
  /// @param callback the function that will return this router's table.
  void OnTableRequest(
      const std::function<const std::map<uint16_t, int16_t> &(void)> &callback);

 private:
  // The rest of the flows as described in the protocol.
  void GetRequestReceived(uint16_t router_id, const Message &m);
  void PushTableReceived(uint16_t router_id, const Message &m);
  void TableResponseReceived(uint16_t router_id, const Message &m);
  void AckReceived(uint16_t router_id, const Message &m);

  std::unique_ptr<MessageIoFactory> message_io_factory_;
  std::unique_ptr<util::TimeoutFactory> timeout_factory_;
  std::atomic<bool> active_;  // Is server active?
  std::atomic<bool> alive_;   // Is spawned thread alive?
  std::function<void(const std::map<uint16_t, int16_t> &)> table_receipt_;
  std::function<const std::map<uint16_t, int16_t> &(void)> table_request_;
};
}  // namespace router

#endif  // ROUTER_MESSAGE_FLOW_H_
