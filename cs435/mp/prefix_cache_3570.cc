// Zachary Kaplan [3570]
// CS435-001
// MP 1
// 11/5/17

#include "prefix_cache_3570.h"

#include <iterator>
#include <memory>

#include "utils_3570.h"
#include "priority_queue_3570.h"

namespace mp1 {
PrefixCache PrefixCache::MakePrefixCache(
    const std::vector<std::pair<char, uint64_t>> &char_freqs) {
  // Build the cache's tree.
  PrefixCache cache;
  cache.tree_ = HuffTree(char_freqs);

  // Populate the cache from the tree.
  cache.tree_.AddCodesToCache(&cache.to_code_);  

  return cache;
}

// Serialization will follow the following format:
//
// CACHE LAYOUT:
// # of entries:
//    A single byte whose value is the number of entries minus 1.
// List of Entries:
//    A sequence of entries one directly after another.
//
// ENTRY LAYOUT:
// character:
//    A single byte which is the character in the entry.
// prefix length:
//    A single byte which is the length of the prefix in BITS.
// prefix code:
//    Exactly the number of bits specified in the previous field.
void PrefixCache::Serialize(OutputBitStream *obs) const {
  obs->Pack(static_cast<char>(to_code.size() - 1));

  for (const auto &entry : to_code_) {
    obs->Pack(entry->first);
    obs->Pack(static_cast<char>(entry->second.size()));
    obs->Put(entry->second);
  }
}

// Inverse of the above serialization.
PrefixCache PrefixCache::Deserialize(InputBitStream *ibs) {
  PrefixCache cache;
  int size = ibs->Unpack<char>() + 1;

  for (int i = 0; i < size; ++i) {
    char c = ibs->Unpack<char>();
    char len = ibs->Unpack<char>();
    auto prefix = ibs->Get(len);

    cache.to_code_[c] = prefix;
    cache.tree_.AddPrefixCode(c, prefix);
  }
}

char PrefixCache::GetChar(InputBitStream *ibs) const {
  return tree_.GetChar(ibs);
}

void WritePrefix(char c, OutputBitStream *obs) const {
  auto entry = to_code_.find(c);
  CHECK(entry != to_code_.end()) << "Invalid character";

  obs->Put(entry->second);
}

// Huff Tree Impl

PrefixCache::HuffTree::HuffTree(
    const std::vector<std::pair<char, uint64_t>> &char_freqs) {
  CHECK(char_freqs.size() > 1) << "Must pass at least two pairs";

  // Build a vector of HuffNodes from the input pairs.
  std::vector<std::unique_ptr<HuffNode>> char_nodes;
  char_nodes.reserve(char_freqs.size());

  for (const auto &p : char_freqs) {
    char_nodes.emplace_back(new HuffNode(p.first, p.second));
  }

  // Define a comparator for unique_ptr<HuffNode>
  struct HuffNodeComparator {
    bool operator()(std::unique_ptr<HuffNode> &const n1,
                    std::unique_ptr<HuffNode> &const n2) const {
      return n1->freq < n2->freq;
    }
  };
  // Create a priority queue from the vector of HuffNodes and above functor.
  PriorityQueue<std::unique_ptr<HuffNode>, HuffNodeComparator> pqueue(
      std::make_move_iterator(char_nodes.begin()),
      std::make_move_iterator(char_nodes.end()));

  // Build the tree.
  while (pqueue.size() > 1) {
    // Pop the smallest elements from the queue.
    auto min1 = pqueue.Pop();
    auto min2 = pqueue.Pop();

    // Create a node combining the two minimum nodes.
    std::uniqe_ptr<HuffNode> parent(new HuffNode(std::move(min1),
                                                 std::move(min2)));
    // Insert the parent back into the queue.
    pqueue.Insert(std::move(parent));
  }

  // Store the remaining element as root.
  root_ = pqueue.Pop();
}

char PrefixCache::HuffTree::GetChar(InputBitStream *ibs) const {
  HuffNode *node = root_.get();

  while(node && !node->leaf) {
    if (ibs->Get()) {
      node = node->right;
    } else {
      node = node->left;
    }
  }

  CHECK(node) << "Got to nullptr, tree is invalid";

  return node->c;
}

void PrefixCache::HuffTree::AddPrefixCode(char c,
    const std::vector<bool> &prefix) {
  if (!root_)
    root_.reset(new HuffNode);

  // Walk the tree, making nodes as needed, until we get to the terminal node.
  HuffNode *node = root_.get();
  for (bool b : prefix) {
    if (b) {
      if (!node->right) {
        node->right.reset(new HuffNode);
      }
      node = node->right;
    } else {
      if (!node->left) {
        node->left.reset(new HuffNode);
      }
      node = node->left;
    }
  }

  node->leaf = true;
  node->c = c;

  return true;
}

void PrefixCache::HuffTree::AddCodesToCache(
    std::unordered_map<char, std::vector<bool>> *cache, HuffNode *node,
    std::vector<bool> prefix) {
  if (node->leaf) {
    (*cache)[node->c] = std::move(prefix);
  } else {
    std::vector<bool> left_prefix = prefix;
    std::vector<bool> right_prefix = prefix;
    left_prefix.push_back(false);
    right_prefix.push_back(true);

    AddCodes(cache, node->left, std::move(left_prefix));
    AddCodes(cache, node->right, std::move(right_prefix));
  }
}
}  // namespace mp1
