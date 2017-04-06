#include <iostream>
#include <map>
#include <thread>

#include "logging.h"
#include "config_parse.h"
#include "message_flow.h"
#include "message_io.h"
#include "socket.h"

#ifdef R0
const uint16_t kRouterId = 0;
#elif R1
const uint16_t kRouterId = 1;
#elif R2
const uint16_t kRouterId = 2;
#elif R3
const uint16_t kRouterId = 3;
#endif

using namespace std;
using router::Configuration;
using router::Client;
using router::Server;
using router::SocketMessageIoFactory;
using util::SimpleTimeoutFactory;

void WaitEnter() {
  while (cin.get() != '\n')
    this_thread::yield();
}

int main(int argc, char **argv) {
  Configuration config(&argc, &argv, kRouterId);

  // Error ocurred during configuration.
  if (config.RetVal()) {
    return config.RetVal();
  }

  INFO << "Configuration processed successfully";
  INFO << "Router R" << kRouterId << "'s intial table is:" << endl
    << config.Dvt();
  INFO << "Press <Enter> to start the main loop.";
  
  WaitEnter();

  INFO << "Starting router R" << kRouterId << "'s main loop...";
  INFO << "Press <Enter> to terminate.";

  // Create Client (takes ownership of raw pointers).
  Client client(new SocketMessageIoFactory(config.Verbose()),
      new SimpleTimeoutFactory);

  // Setup Server Callbacks.
  Server::OnTableReceipt([&config, &client](uint16_t source_id,
        map<uint16_t, int16_t> table) {
      // If table was updated, broadcast tables.
      INFO << "Received the following table from R" << source_id << ":"
        << endl << table;
      if (config.Dvt().UpdateTable(source_id, table)) {
        INFO << "Updated table based on message from R" << source_id << ":"
          << endl << config.Dvt();

        client.BroadcastTable(config.Neighbors(),
            static_cast<map<uint16_t, int16_t>>(config.Dvt().Table()));
      } else {
        INFO << "No table update was necessary based on the message from R"
          << source_id << endl;
      }
    });
  Server::OnTableRequest([&config]() -> map<uint16_t, int16_t> {
      return config.Dvt().Table();
    });

  // Start Server (takes ownership of raw pointers).
  Server server(new SocketMessageIoFactory(config.Verbose()),
      new SimpleTimeoutFactory);

  // Send initial Broadcast
  client.BroadcastTable(config.Neighbors(),
      static_cast<map<uint16_t, int16_t>>(config.Dvt().Table()));

  WaitEnter();

  INFO << "Shutting down router R" << kRouterId;

  return 0;
}
