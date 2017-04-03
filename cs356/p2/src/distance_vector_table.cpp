#include "distance_vector_table.h"

#include <iomanip>

using namespace std;

namespace router {
DistanceVectorTable::DistanceVectorTable(uint16_t source_router_id) {
  table_[source_router_id] = 0;
}

bool DistanceVectorTable::AddEdge(uint16_t router_id, int16_t distance) {
  auto res = table_.emplace(router_id, distance);
  return res.second;
}

bool DistanceVectorTable::UpdateTable(uint16_t source_id,
    const map<uint16_t, int16_t> &table) {
  // Lookup source_id.
  auto it = table_.find(source_id);
  if (it == table_.end())
    return false;

  // Distance to source_id.
  int16_t prepend = it->second;
  // Whether or not the internal table was updated.
  bool edited = false;
  
  for (const auto &entry : table) {
    int16_t distance = prepend + entry.second;
    // First try inserting the new edge (path doesn't already exist).
    auto res = table_.emplace(entry.first, distance);
    if (res.second) {  // Successful insert.
      edited = true;
    } else {  // Edge already existed.
      // Compare the calculated distance to the current value, update if needed.
      if (distance < res.first->second) {
        res.first->second = distance;
        edited = true;
      }
    }
  }

  return edited;
}

ostream &operator<<(ostream &o, const DistanceVectorTable &dvt) {
  const char *kBreak = "+--------+--------+";
  o << "   ROUTING TABLE   " << endl;
  o << kBreak << endl;
  o << "| Router |  Dist  |" << endl;
  o << kBreak << endl;

  for (const auto &entry : dvt.Table()) {
    o << "| " << setw(6) << entry.first
      << " | " << setw(6) << entry.second << " |" << endl;
  }

  return o << kBreak << endl;
}
}  // namespace router
