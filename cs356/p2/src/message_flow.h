#ifndef ROUTER_MESSAGE_FLOW_H_
#define ROUTER_MESSAGE_FLOW_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>

#include "message.h"
#include "message_io.h"
#include "thread_pool.h"
#include "timeout.h"

namespace router {
/// Resets all active "connections." Essentially clears all active timeouts.
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
  /// @param max_threads maximum number of threads to dedicate towards client
  ///                    requests.
  Client(MessageIoFactory *message_io_factory,
      util::TimeoutFactory *timeout_factory, int max_threads = 4);

  /// Sends a Get Table Request message to the router specified.
  ///
  /// @param router_id the router to send the request to.
  void SendGetTableRequest(uint16_t router_id);

  /// Sends a Push Table message containing the specified table to the router
  /// specified.
  ///
  /// @param router_id the router to send the request to.
  /// @param table a const reference to this router's routing table.
  void PushTableTo(uint16_t router_id,
      const std::map<uint16_t, int16_t> &table);

  /// Broadcasts table using PushTable to all routers specified.
  ///
  /// @param neighbor_ids the routers to send the request to.
  /// @param table a const reference to this router's routing table.
  void BroadcastTable(std::set<uint16_t> neighbor_ids,
      const std::map<uint16_t, int16_t> &table);

 private:
  std::unique_ptr<MessageIoFactory> message_io_factory_;
  std::unique_ptr<util::TimeoutFactory> timeout_factory_;
  util::ThreadPool pool_;
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
  /// @param max_threads the maximum number of threads to dedicate to serving
  ///                    incoming requests.
  Server(MessageIoFactory *message_io_factory,
      util::TimeoutFactory *timeout_factory, int max_threads = 4);
  ~Server();
  
  /// Attaches callback to the receipt of a table event. Will be called with
  /// sender id and received table. New callbacks replace old ones.
  /// NOTE: callback may be called from multiple threads simultaneously.
  ///
  /// @param callback the function to call. Set to default constructed function
  ///                 to remove the callback (default).
  static void OnTableReceipt(
      const std::function<void(uint16_t source_id,
        std::map<uint16_t, int16_t> table)> &callback =
      std::function<void(uint16_t, std::map<uint16_t, int16_t>)>());

  /// Attaches callback to the request for a table. Will be called with no args.
  /// New callbacks replace old ones.
  /// NOTE: callback may be called from multiple threads simultaneously.
  ///
  /// @param callback the function to call. Set to default constructed function
  ///                 to remove the callback (default).
  static void OnTableRequest(
      const std::function<std::map<uint16_t, int16_t>(void)> &callback = 
      std::function<std::map<uint16_t, int16_t>(void)>());

 private:
  // The rest of the flows as described in the protocol.
  void GetRequestReceived(uint16_t router_id, const Message &m);
  void PushTableReceived(uint16_t router_id, const Message &m);
  void TableResponseReceived(uint16_t router_id, const Message &m);
  void AckReceived(uint16_t router_id, const Message &m);

  static std::function<void(uint16_t, std::map<uint16_t, int16_t>)> receipt_cb_;
  static std::mutex receipt_cb_mutex_;
  static std::function<std::map<uint16_t, int16_t>(void)> request_cb_;
  static std::mutex request_cb_mutex_;
  std::unique_ptr<MessageIoFactory> message_io_factory_;
  std::unique_ptr<util::TimeoutFactory> timeout_factory_;
  util::ThreadPool pool_;
  std::atomic<bool> active_;
};
}  // namespace router

#endif  // ROUTER_MESSAGE_FLOW_H_
