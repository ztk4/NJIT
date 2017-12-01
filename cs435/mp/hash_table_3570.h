// Zachary Kaplan [3570]
// CS435-001
// MP2
// 11/30/17

#ifndef MP2_HASH_TABLE_H_
#define MP2_HASH_TABLE_H_

#include <memory>
#include <utility>

namespace mp2 {
// A hash table with external key storage: only values are internally stored in
// the table. HOWEVER, this means there must be away to retrieves the key
// associated with a value. This logic is handled by KeyFunctor.
// Assumes both Key and Value are trivially copyable.
// Uses naive quadratic probing for collision resolution.
template <typename Key,           // The type used to key into the table.
          typename Value,         // The type of the value stored in the table.
          typename HashFunctor,   // A functor that hashes a key.
          typename KeyFunctor>    // A functor that retrieves keys from an
                                  // instance of the value type.
class HashTable {
 public:
  // Creates an empty hash table with the specified capacity.
  explicit HashTable(size_t capacity, HashFunctor hasher = HashFunctor(),
                     KeyFunctor get_key = KeyFunctor())
    : hasher_(hasher),
      get_key_(get_key),
      capacity_(capacity),
      table_(new Entry[capacity]) {}

  // Disallow copy and assign.
  HashTable(const HashTable<Key, Value, HashFunctor, KeyFunctor> &) = delete;
  HashTable<Key, Value, HashFunctor, KeyFunctor> &operator=(
      const HashTable<Key, Value, HashFunctor, KeyFunctor> &) = delete;
  // Allow move operations.
  HashTable(HashTable<Key, Value, HashFunctor, KeyFunctor> &&) = default;
  HashTable<Key, Value, HashFunctor, KeyFunctor> &operator=(
      HashTable<Key, Value, HashFunctor, KeyFunctor> &&) = default;

  // Boolean size functions.
  bool Empty() const { return size_ == 0; }
  bool Full() const { return size_ == capacity_; }
  // Size getters.
  size_t Size() const { return size_; }
  size_t Capacity() const { return capacity_; }

  // Sets the capactity of the table, returns true on success.
  bool SetCapacity(size_t capacity) {
    if (capacity < size_) return false;      // Not possible.
    if (capacity == capacity_) return true;  // Already done.

    // Setting the capacity involves re-inserting every element, so we might
    // as well make a new HashTable, insert into it, then move it to this one.
    HashTable<Key, Value, HashFunctor, KeyFunctor> new_table(capacity,
                                                             hasher_,
                                                             get_key_);

    // Go through each entry, and insert each as needed to the new table.
    for (Entry *e = table_.get(); e != table_.get() + capacity_; ++e) {
      // We only have to re-insert used entries.
      if (e->state == Entry::kFull) {
        bool success = new_table.Insert(get_key_(e->value), e->value);
        // Unable to make a hash table from this one with this capacity.
        if (!success) return false;
      }
    }

    // Move the new table into this one.
    *this = std::move(new_table);
    return true;
  }

  // Data accessors.
  // Returns (true, value) if found, or (false, ***) if not found.
  std::pair<bool, Value> Search(Key key) const {
    auto hash = hasher_(key) % capacity_;
    
    // Try searching up to capacity times.
    for (size_t i = 0; i < capacity_; ++i) {
      const auto &entry = table_[hash];
      if (entry.state == Entry::kOpen) return std::make_pair(false, Value());
      if (entry.state == Entry::kFull && key == get_key_(entry.value)) {
        return std::make_pair(true, entry.value);
      }

      // Quadratically probe.
      // NOTE: Adding sequential odd numbers to hash is the same as recomputing
      //       the hash each time and adding i^2.
      hash += i + i + 1;
      hash %= capacity_;
    }

    return std::make_pair(false, Value());
  }
  // Returns true on success. (Fails only when key is already in the map).
  bool Insert(Key key, Value value) {
    auto hash = hasher_(key) % capacity_;

    // Pointer to the entry to insert into, initially null.
    Entry *dest = nullptr;

    // Only try inserting if there is space.
    if (size_ < capacity_) {
      // Try inserting up to capacity times.
      for (size_t i = 0; i < capacity_; ++i) {
        auto &entry = table_[hash];
        if (entry.state == Entry::kOpen) {
          // If dest is already set (to a deleted cell), don't reset it.
          if (!dest) dest = &entry;
          // Good to insert into dest.
          break;
        }
        if (entry.state == Entry::kDeleted) {
          // Can't break right away, must make sure key isn't already in table.
          // Only set dest if unset.
          if (!dest) dest = &entry;
        } else if (key == get_key_(entry.value)) {  // Full and matches.
          return false;  // Can't insert, element already in table.
        }

        // Quadratic Probing.
        hash += i + i + 1;
        hash %= capacity_;
      }
    }

    if (!dest) {
      // Capacity isn't large enough, double it!
      bool success = SetCapacity(2 * capacity_);
      if (!success) return false;
      // Retry insert.
      return Insert(key, value);
    }

    // Preform an insertion at dest.
    dest->state = Entry::kFull;
    dest->value = value;
    ++size_;

    return true;
  }
  // Returns (true, value) on success, and (false, ***) when key is not found.
  std::pair<bool, Value> Delete(Key key) {
    auto hash = hasher_(key) % capacity_;

    // Try deleting up to capacity times.
    for (size_t i = 0; i < capacity_; ++i) {
      auto &entry = table_[hash];
      if (entry.state == Entry::kOpen) return std::make_pair(false, Value());
      if (entry.state == Entry::kFull && key == get_key_(entry.value)) {
        entry.state = Entry::kDeleted;
        --size_;
        return std::make_pair(true, entry.value);
      }

      // Quadratic Probing.
      hash = i + i + 1;
      hash %= capacity_;
    }

    return std::make_pair(false, Value());
  }

 private:
  // Simple entry type.
  struct Entry {
    Value value;
    enum State { kOpen, kFull, kDeleted } state = kOpen;
  };

  // Functor instances.
  HashFunctor hasher_;
  KeyFunctor get_key_;
  // Size information.
  size_t size_ = 0;
  size_t capacity_;
  // The table itself.
  std::unique_ptr<Entry[]> table_;
};
}  // namespace mp2

#endif  // MP2_HASH_TABLE_H_
