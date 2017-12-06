// Zachary Kaplan [3570] CS435-001 MP
// MP 2
// 11/30/17

#ifndef MP2_LEXICON_H_
#define MP2_LEXICON_H_

#include <iostream>
#include <memory>

#include "hash_table_3570.h"

namespace mp2 {
class Lexicon {
 public:
  explicit Lexicon(size_t capacity);
  // Disallow copy and assign.
  Lexicon(const Lexicon &) = delete;
  Lexicon &operator=(const Lexicon &) = delete;
  // Disallow move operations.
  Lexicon(Lexicon &&) = delete;
  Lexicon &operator=(Lexicon &&) = delete;

  // Boolean size accessors.
  bool Empty() const { return table_.Empty(); }
  bool Full() const { return table_.Full(); }
  
  // Size accessors.
  size_t Size() const { return table_.Size(); }
  size_t Capacity() const { return table_.Capacity(); }
  size_t ArenaSize() const { return arena_size_; }

  // Size setters.
  bool SetCapacity(size_t capacity) { return table_.SetCapacity(capacity); }
  bool SetArenaSize(size_t arena_size);

  // Returns the hash_table index of the str if found, otherwise -1.
  ssize_t Search(const char *str) const;
  // Returns the hash_table index of the str if inserted, otherwise -1.
  ssize_t Insert(const char *str);
  // Returns the hash_table index of the str if deleted, otherwise -1.
  ssize_t Delete(const char *str);

  // Prints this lexicon's arena and hash table.
  void Print(std::ostream *os) const;

 private:
  // Predicate for comparing keys.
  struct StringEq {
    bool operator()(const char *str1, const char *str2) const;
  };
  // Hashing functor for char *.
  struct StringHasher {
    size_t operator()(const char *str) const;
  };
  // Key retrieval functor for values (indices into arena).
  struct GetKey {
    explicit GetKey(const Lexicon *lexicon) : lexicon(lexicon) {}
    // Default Copy and Assign.
    GetKey(const GetKey &) = default;
    GetKey &operator=(const GetKey &) = default;

    const char *operator()(size_t idx) const;

    // Pointer to the lexicon owning the hash table.
    const Lexicon *lexicon;
  };

  // The hash table.
  HashTable<const char *, size_t, StringEq, StringHasher, GetKey> table_;
  // The size of the arena in bytes.
  size_t arena_size_;
  // The arena (place where strings are allocated).
  std::unique_ptr<char[]> arena_;
  // Index of the next available byte in the arena.
  size_t arena_base_;

  // Make GetKey a friend so it may access the arena directly.
  friend struct GetKey;
};
}  // namespace mp2

#endif  // MP2_LEXICON_H_
