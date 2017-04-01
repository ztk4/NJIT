#ifndef ROUTER_DISTANCE_VECTOR_TABLE_H_
#define ROUTER_DISTANCE_VECTOR_TABLE_H_

#include <cstdint>
#include <iostream>
#include <map>

namespace router {
class DistanceVectorTable {
 public:
  /// Constructs a new table with the specified router as the source.
  ///
  /// @param source_router_id the router id of this router. Will insert a 
  ///                         self referencial edge with weight 0.
  explicit DistanceVectorTable(uint16_t source_router_id);
  // Disallow Copy & Assign
  DistanceVectorTable(const DistanceVectorTable &) = delete;
  DistanceVectorTable &operator=(const DistanceVectorTable &) = delete;

  /// Adds an edge to this table to the specified router with the given
  /// distance if an edge to that router does not already exist.
  ///
  /// @param router_id the id of the router to add an edge to.
  /// @param distance the weight of the edge.
  ///
  /// @returns true if the edge was added, and false if the edge already exists.
  bool AddEdge(uint16_t router_id, int16_t distance);

  /// @returns a const reference to the mapping table.
  const std::map<uint16_t, int16_t> &Table() const;

  /// Updates this table with new edges from a specified known source.
  ///
  /// @param source_id the router id of the source of this new table.
  /// @param table that routers table.
  ///
  /// @returns true if any update was made to this table, false otherwise.
  bool UpdateTable(uint16_t source_id,
      const std::map<uint16_t, int16_t> &table);

 private:
  std::map<uint16_t, int16_t> table_;
};

/// Printing operator overload for DistanceVectorTable.
std::ostream &operator<<(std::ostream &o, const DistanceVectorTable &dvt);
}  // namespace router

#endif  // ROUTER_DISTANCE_VECTOR_TABLE_H_
