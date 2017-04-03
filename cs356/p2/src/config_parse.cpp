#include "config_parse.h"

#include <cctype>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <limits>

#include "message_flow.h"
#include "message_io.h"

using namespace std;

namespace router {
namespace {
const char *kUsageFormat = 
  R"(Usage: %s configuration.cfg [flags]
        
        Supported Flags:

        -v --verbose    Print Message Traces
)";

enum Status {
  OK = 0,
  TOO_FEW_ARGS = -1,
  CANNOT_OPEN_CFG_FILE = -2,
  CFG_PARSE_ERROR = -3,
  UNKNOWN_FLAG = -4,
};
}  // anonymous namespace
// Fills in the configuration object.
Configuration::Configuration(int *argc_p, char ***argv_p,
    uint16_t source_router_id)
    : source_router_id_(source_router_id),
      dvt_(source_router_id) {
  --(*argc_p);
  char *prgm_name = **argv_p;

  // First argument is required (config file).
  if (!*argc_p) {
    fprintf(stderr, "Error: Expected at least one argument.\n");
    fprintf(stderr, kUsageFormat, prgm_name);
    ret_val_ = TOO_FEW_ARGS;
    return;
  }

  // Try using first arg as config file name.
  --(*argc_p);
  ++(*argv_p);
  ifstream cfg(**argv_p);
  if (!cfg) {
    fprintf(stderr, "Error: Config file '%s' not found.\n", **argv_p);
    fprintf(stderr, kUsageFormat, prgm_name);
    ret_val_ = CANNOT_OPEN_CFG_FILE;
    return;
  }

  // Parse config file.
  ParseCfg(cfg);
  // If an error ocurred while parsing.
  if (ret_val_)
    return;

  while (*argc_p) {
    ++(*argv_p);
    if (!strcmp(**argv_p, "-v") || !strcmp(**argv_p, "--verbose")) {
      verbose_ = true;
    } else {  // Unrecognized Flag.
      fprintf(stderr, "Error: Unknown flag '%s'\n", **argv_p);
      fprintf(stderr, kUsageFormat, prgm_name);
      ret_val_ = UNKNOWN_FLAG;
      return;
    }

    --(*argc_p);
  }
}
// Tidies up resources.
Configuration::~Configuration() {
  // Clean up Socket
  SocketMessageIo::CloseSocket();
  // Clear pending flows
  ResetAll();
}

void Configuration::ParseCfg(ifstream &cfg) {
  int current_line = 1;
  bool declaring_routers = true;

  while (cfg) {
    current_line += IgnoreWhiteSpaceAndComment(cfg);

    // Parse a router descriptor.
    if (cfg.get() != 'R') {
      ret_val_ = CFG_PARSE_ERROR;
    }
    uint16_t router_id = ParseInt(cfg);
    if (ret_val_) {
      fprintf(stderr, "Error: Malformed Router Descriptor on line %d\n",
          current_line);
      return;
    }

    // Ensure there is at least one space character.
    if (!isspace(cfg.peek())) {
      fprintf(stderr, "Error: Expected delimitting white space character on"
          " line %d\n", current_line);
      ret_val_ = CFG_PARSE_ERROR;
      return;
    }
    // Ignore space between first and second term on line.
    if (IgnoreWhiteSpaceAndComment(cfg)) {
      fprintf(stderr, "Error: Premature EOL while parsing line %d\n",
          current_line);
      ret_val_ = CFG_PARSE_ERROR;
      return;
    }

    // Format is R* An.IPv4.Address.With:PORT
    if (declaring_routers) {
      // Transition to second half of file (network topology).
      if (cfg.peek() == 'R') {
        declaring_routers = false;
      } else {
        util::InSocket::InAddress addr = ParseIPv4(cfg);
        if (ret_val_) {
          fprintf(stderr, "Error: Malformed IPv4 Address on line %d\n",
              current_line);
          return;
        }

        // Add address pair.
        SocketMessageIo::AddRouterAddressPair(router_id, addr);
        // If this is the address of this router, create socket.
        if (router_id == source_router_id_) {
          util::Socket *socket = new util::InSocket(util::InSocket::UDP);
          socket->Bind(&addr);
          SocketMessageIo::SetSocket(socket);  // Takes Ownership.
        }
      }
    }

    // Format is R* R+ Weight
    if (!declaring_routers) {
      if (cfg.get() != 'R') {
        ret_val_ = CFG_PARSE_ERROR;
      }
      uint16_t second_router_id = ParseInt(cfg);
      if (ret_val_) {
        fprintf(stderr, "Error: Malformed Router Descriptor on line %d\n",
            current_line);
        return;
      }

      // Ensure there is at least one space character.
      if (!isspace(cfg.peek())) {
        fprintf(stderr, "Error: Expected delimitting white space character on"
            " line %d\n", current_line);
        ret_val_ = CFG_PARSE_ERROR;
        return;
      }
      // Ignore space between second router and edge weight.
      if (IgnoreWhiteSpaceAndComment(cfg)) {
        fprintf(stderr, "Error: Premature EOL while parsing line %d\n",
            current_line);
        ret_val_ = CFG_PARSE_ERROR;
        return;
      }

      uint16_t edge_weight = ParseInt(cfg);
      if (ret_val_) {
        fprintf(stderr, "Error: Malformed Edge Weight on line %d\n",
            current_line);
        return;
      }

      // If source_router_id is either of the routers, add the appropriate edge.
      if (router_id == source_router_id_) {
        dvt_.AddEdge(second_router_id, edge_weight);
        neighbors_.insert(second_router_id);
      } else if (second_router_id == source_router_id_) {
        dvt_.AddEdge(router_id, edge_weight);
        neighbors_.insert(router_id);
      }
    }

    // Ignore the rest of the line at least, ensure at least one new line if
    // not at EOF
    int lines_ignored = IgnoreWhiteSpaceAndComment(cfg);
    if (!lines_ignored && cfg) {
      fprintf(stderr, "Error: Expected new line while parsing line %d\n",
          current_line);
      ret_val_ = CFG_PARSE_ERROR;
      return;
    }

    current_line += lines_ignored;
  }
}

// Returns lines parsed.
int Configuration::IgnoreWhiteSpaceAndComment(ifstream &cfg) {
  char c;
  int line = 0;

  do {
    // Ignore spaces.
    while (isspace(c = cfg.get()))
      if (c == '\n') ++line;
    // Comment, ignore the rest of the line.
    if (c == '#') {
      cfg.ignore(numeric_limits<streamsize>::max(), '\n');
      ++line;
    } else { 
      break;
    }
  } while (true);

  cfg.putback(c);
  return line;
}

uint16_t Configuration::ParseInt(ifstream &cfg) {
  char digits[6], *ch = digits;

  while (isdigit(*ch = cfg.get())) {
    if (ch - digits == 6) {  // Number too big.
      ret_val_ = CFG_PARSE_ERROR;
      return 0;
    }
    ++ch;
  }

  cfg.putback(*ch);
  *ch = '\0';  // Null Terminating Character.
  int res = atoi(digits);
  if (res > (1 << 16) - 1) { // Too Big.
    ret_val_ = CFG_PARSE_ERROR;
    return 0;
  }

  return (uint16_t) res;
}

util::InSocket::InAddress Configuration::ParseIPv4(ifstream &cfg) {
  uint16_t addr[4], port;

  addr[0] = ParseInt(cfg);
  for (int i = 1; !ret_val_ && i < 4; ++i) {
    if (cfg.get() != '.') {
      ret_val_ = CFG_PARSE_ERROR;
      return util::InSocket::InAddress();
    } else {
      addr[i] = ParseInt(cfg);
    }
  }

  if (cfg.get() != ':') {
    ret_val_ = CFG_PARSE_ERROR;
    return util::InSocket::InAddress();
  } else {
    port = ParseInt(cfg);
  }

  return util::InSocket::InAddress(
      (addr[0] & 0xFF)        |
      (addr[1] & 0xFF) << 8   |
      (addr[2] & 0xFF) << 16  |
      (addr[3] & 0xFF) << 24,
      port);
}
}  // namespace router
