#include <map>
#include <iostream>
#include <cstdint>
#include <thread>
#include <vector>
#include <cstring>

#include "socket.h"
#include "message_flow.h"
#include "message_io.h"

#define IP_ADDR(a, b, c, d) (\
  ((a & 0xFF) << 24) |\
  ((b & 0xFF) << 16) |\
  ((c & 0xFF) << 8)  |\
  ((d & 0xFF))\
)

using namespace router;
using namespace util;
using namespace std;

const uint16_t kPort = 0x4242;

#ifdef R0
map <uint16_t, int16_t> routing_table = {
  {0, 0}, {1, 1}, {2, 3}, {3, 7},
};
uint16_t my_id = 0;
#elif R1
map <uint16_t, int16_t> routing_table {
 {0, 1}, {1, 0}, {2, 1},
};
uint16_t my_id = 1;
#endif
// For Request Counting
atomic<int> receive_count(0), request_count(0);

ostream &operator<<(ostream &o, const map<uint16_t, int16_t> &table) {
  o << "Routing Table (missing entries are unroutable):" << endl;
  for (const auto &entry : table) {
    o << "\tR" << entry.first << ":\t" << entry.second << endl;
  }
  return o;
}

int main(int argc, char **argv) {
  bool verbose = false;
  if (argc == 2) {
    if (!strncmp(argv[1], "-v", 3))
      verbose = true;
    else
      return -2;
  } else if (argc > 2) {
    return -1;
  }

  InSocket::InAddress r0(IP_ADDR(128, 235, 209, 204), kPort);
  InSocket::InAddress r1(IP_ADDR(128, 235, 209, 205), kPort);

  // Initialize all addresses.
  SocketMessageIo::AddRouterAddressPair(0, r0);
  SocketMessageIo::AddRouterAddressPair(1, r1);

  Socket *main_socket = new InSocket(InSocket::UDP);
#ifdef R0
  main_socket->Bind(&r0);
#elif R1
  main_socket->Bind(&r1);
#endif
  SocketMessageIo::SetSocket(main_socket);  // Takes ownership.

  // Spawn servers
  Server *servers[4];;
  for (int i = 0; i < 4; ++i) {
    servers[i] = new Server(new SocketMessageIoFactory(verbose),
        new SimpleTimeoutFactory);
    servers[i]->OnTableReceipt(
        [](const map<uint16_t, int16_t> &table) {
          cout << table << endl;
          ++receive_count;
        });
    servers[i]->OnTableRequest(
        []() {
          ++request_count;
          return routing_table;
        });
  }

  Client client(new SocketMessageIoFactory(verbose), new SimpleTimeoutFactory);

#ifdef R0
  cout << "My initial routing Table:" << endl;
  cout << routing_table << endl;

  client.SendGetTableRequest(1);
  while (receive_count < 1)
    this_thread::yield();

  client.PushTableTo(1, routing_table);
#elif R1
  cout << "My initial routing Table:" << endl;
  cout << routing_table << endl;

  while (receive_count < 1 && request_count < 1)
    this_thread::yield();
#endif

  // Make sure last requests are handled before dying
  this_thread::sleep_for(chrono::seconds(5));

  // Kill Servers
  for (int i = 0; i < 4; ++i)
    delete servers[i];

  return 0;
}
