#ifndef ROUTER_CONFIG_PARSE_H_
#define ROUTER_CONFIG_PARSE_H_

#include <cstdint>
#include <fstream>
#include <set>

#include "distance_vector_table.h"
#include "socket.h"

// The expected configuration file is of the following format:
//
// R* An.IPv4.Address.With:Port  # Router entry maps ID (*) to address and port.
// { ... More Entries ... }
// R* R+ Weight  # Adds an edge to the network from R* to R+ with Weight.
// { ... More Edges ... }
//
// Where Everything after a '#' character on a line is ignored. E.g.
//
// # Routers in my network
// R0 128.230.10.4:12523
// R1 128.230.10.6:16243
// 
// # Edges in my network
// R0 R1 7

namespace router {
/// Initializes a router's configuration based on command line args and a passed
/// in source_router_id. 
class Configuration {
 public:
  /// Constructs a configuration from command line args and source_router_id.
  ///
  /// @param argc_p a pointer to argc (edits argc for all consumed args).
  /// @param argv_p a pointer to argv (edits argv for all consumed args).
  /// @param source_router_id this router's id. 
  Configuration(int *argc_p, char ***argv_p, uint16_t source_router_id);
  // Disallow Copy & Assign
  Configuration(const Configuration &) = delete;
  Configuration &operator=(const Configuration &) = delete;
  ~Configuration();

  /// Getters for fields
  const std::set<uint16_t> &Neighbors() const { return neighbors_; }
  const DistanceVectorTable &Dvt() const { return dvt_; }
  bool Verbose() const { return verbose_; }
  int RetVal() const { return ret_val_; }

  /// Non-const getter for dvt (needs to update ocassionaly).
  DistanceVectorTable &Dvt() { return dvt_; }

 private:
  void ParseCfg(std::ifstream &cfg);
  int IgnoreWhiteSpaceAndComment(std::ifstream &cfg);
  uint16_t ParseInt(std::ifstream &cfg);
  util::InSocket::InAddress ParseIPv4(std::ifstream &cfg);

  uint16_t source_router_id_;
  std::set<uint16_t> neighbors_;
  DistanceVectorTable dvt_; 
  bool verbose_ = false;
  int ret_val_ = 0;
};
}  // namespace router

#endif  // ROUTER_CONFIG_PARSE_H_
