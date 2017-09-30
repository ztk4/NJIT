// Zachary Kaplan [3570]
// CS435-001
// MP 1
// 9/29/17

#ifndef MP1_HUFF_TREE_H_
#define MP1_HUFF_TREE_H_

#include <cstdint>
#include <iterator>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>

#include "utils_3570.h"

// Temporary forward declarations.
class InputBitStream;
class OutputBitStream;

namespace mp1 {
class HuffTree {
 public:
  // Consturcts a Huffman Tree from a vector of (char, freq) pairs.
  explicit HuffTree(const std::vector<std::pair<char, uint64_t>> &char_freqs);

  // Default move constructor/assignment.
  HuffTree(HuffTree &&) = default;
  HuffTree &operator=(HuffTree &&) = default;

  // Returns a HuffTree from the serialized data starting at the leading byte of
  // bits.
  static HuffTree Deserialize(InputBitStream *bits);

  // Serializes this tree to stream bits.
  void Serialize(OutputBitStream *bits) const;

  // Gets the character associated with a prefix.
  char GetChar(const std::vector<bool> &prefix) const;
  // Gets the character associated with a prefix of unknown length from a
  // character stream.
  char GetChar(InputBitStream *bits) const;

  // Gets the prefix associated with a character.
  std::vector<bool> GetPrefix(char c) const;

 private:
  // Private default constructor. Used for deserializing trees from streams.
  HuffTree();
  // Internal Node Structure
  struct HuffNode {
    // Default Constructor for internal nodes while reconstructing a serialized
    // tree from a stream.
    HuffNode() : leaf(false), c(0), left(nullptr), right(nullptr), freq(0) {}
    // Leaf Node Constructor
    explicit HuffNode(char c, uint64_t freq = 0)
      : leaf(true),
        c(c),
        left(nullptr),
        right(nullptr),
        freq(freq) {}
    // Internal Node Constructor
    // NB: left and right are non-null.
    HuffNode(std::unique_ptr<HuffNode> left, std::unique_ptr<HuffNode> right)
      : leaf(false),
        c(0),
        left(std::move(NOTNULL(left))),
        right(std::move(NOTNULL(right))),
        freq(left->freq + right->freq) {}

    // Default move constructor/assignment.
    HuffNode(HuffNode &&) = default;
    HuffNode &operator=(HuffNode &&) = default;

    // Whether or not this Node represents a single character.
    bool leaf;
    // Character for leaf nodes (otherwise useless).
    char c;
    // Children in the tree for internal nodes (otherwise useless).
    std::unique_ptr<HuffNode> left;
    std::unique_ptr<HuffNode> right;
    // Freqeuncy (Either of this character, or sum of children).
    uint64_t freq;
  };

  // The root of this huff tree.
  std::unique_ptr<HuffNode> root_;
};
}  // namespace mp1

#endif  // MP1_HUFF_TREE_H_
