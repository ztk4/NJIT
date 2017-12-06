// Zachary Kaplan [3570] CS435-001 MP
// MP 1
// 9/29/17

#ifndef MP1_PREFIX_CACHE_H_
#define MP1_PREFIX_CACHE_H_

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <utility>

#include "bit_stream_3570.h"
#include "utils_3570.h"

namespace mp1 {
// Forward Declarations.
class HuffTree;

// TODO: If I ever feel motivated, clean this up. This is organized terribly.
class PrefixCache {
 public:
  // Makes a new optimal PrefixCache from a list of characters and their
  // frequencies. (Implementation uses huffman encodings).
  static PrefixCache Make(
      const std::vector<std::pair<char, uint64_t>> &char_freqs);
  // Creates a prefix cache from a serialization in a stream.
  static PrefixCache Deserialize(InputBitStream *ibs);

  // Serializes this cahce to an output stream.
  void Serialize(OutputBitStream *obs) const;

  // Gets the character associated with the prefix code where the input stream
  // points to.
  char GetChar(InputBitStream *ibs) const;

  // Writes the prefix associated with a character to the passed stream.
  // Returns the number of bits written.
  int WritePrefix(char c, OutputBitStream *obs) const;

 private:
  PrefixCache() = default;

  // A Huffman Tree used for implementing the prefix cache.
  class HuffTree {
   public:
    HuffTree() = default;
    // Consturcts a Huffman Tree from a vector of (char, freq) pairs.
    explicit HuffTree(const std::vector<std::pair<char, uint64_t>> &char_freqs);

    // Default move constructor/assignment.
    HuffTree(HuffTree &&) = default;
    HuffTree &operator=(HuffTree &&) = default;

    // Gets the character associated with the prefix code where the input stream
    // points to.
    char GetChar(InputBitStream *ibs) const;

    // Adds a character with a given prefix to the tree.
    // Useful when deserializing a cache.
    void AddPrefixCode(char c, const std::vector<bool> &prefix);

    // Recursive walk that adds all codes to a prefix cache's cache.
    void AddCodesToCache(
        std::unordered_map<char, std::vector<bool>> *cache) const;

   private:
    // Internal Node Structure
    struct HuffNode {
      // Default Constructor for internal nodes while reconstructing a
      // serialized tree from a stream.
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
          freq(this->left->freq + this->right->freq) {}

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

    void RecursiveAddCodesToCache(
        std::unordered_map<char, std::vector<bool>> *cache,
        HuffNode *node, std::vector<bool> prefix) const;

    // The root of this huff tree.
    std::unique_ptr<HuffNode> root_;
  };

  HuffTree tree_;
  std::unordered_map<char, std::vector<bool>> to_code_;
};
}  // namespace mp1

#endif  // MP1_PREFIX_CACHE_H_
