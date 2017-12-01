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
          typename KeyPred,       // A functor that compares keys.
          typename HashFunctor,   // A functor that hashes a key.
          typename KeyFunctor>    // A functor that retrieves keys from an
                                  // instance of the value type.
class HashTable {
 public:
  // Creates an empty hash table with the specified capacity.
  explicit HashTable(size_t capacity, KeyPred pred = KeyPred(),
                     HashFunctor hasher = HashFunctor(),
                     KeyFunctor get_key = KeyFunctor())
    : pred_(pred),
      hasher_(hasher),
      get_key_(get_key),
      capacity_(capacity),
      table_(new Entry[capacity]) {}

  // Disallow copy and assign.
  HashTable(
      const HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> &) = delete;
  HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> &operator=(
      const HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> &) = delete;
  // Allow move operations.
  HashTable(
      HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> &&) = default;
  HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> &operator=(
      HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> &&) = default;

  // Simple entry type.
  struct Entry {
    Value value;
    enum State { kOpen, kFull, kDeleted } state = kOpen;
  };

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
    HashTable<Key, Value, KeyPred, HashFunctor, KeyFunctor> new_table(
        capacity, pred_, hasher_, get_key_);

    // Go through each entry, and insert each as needed to the new table.
    bool success = ForEach([this, &new_table] (const Entry *e) {
      // We only have to re-insert used entries.
      if (e->state == Entry::kFull) {
        return new_table.Insert(get_key_(e->value), e->value).first;
      }

      return true;
    });
    // Unable to resize the hash table to this capacity.
    if (!success) return false;

    // Move the new table into this one.
    *this = std::move(new_table);
    return true;
  }

  // Data accessors.
  // Returns (true, entry) if found, or (false, null) if not found.
  std::pair<bool, const Entry *> Search(Key key) const {
    auto hash = hasher_(key) % capacity_;
    
    // Try searching up to capacity times.
    for (size_t i = 0; i < capacity_; ++i) {
      const auto &entry = table_[hash];
      if (entry.state == Entry::kOpen) return std::make_pair(false, nullptr);
      if (entry.state == Entry::kFull && pred_(key, get_key_(entry.value))) {
        return std::make_pair(true, &entry);
      }

      // Quadratically probe.
      // NOTE: Adding sequential odd numbers to hash is the same as recomputing
      //       the hash each time and adding i^2.
      hash += i + i + 1;
      hash %= capacity_;
    }

    return std::make_pair(false, nullptr);
  }
  // Returns (true, entry) on success, and (false, null) if key already exists.
  std::pair<bool, const Entry *> Insert(Key key, Value value) {
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
        } else if (pred_(key, get_key_(entry.value))) {  // Full and matches.
          return std::make_pair(false, nullptr);
        }

        // Quadratic Probing.
        hash += i + i + 1;
        hash %= capacity_;
      }
    }

    if (!dest) {
      // Capacity isn't large enough, double it!
      bool success = SetCapacity(2 * capacity_);
      if (!success) return std::make_pair(false, nullptr);
      // Retry insert.
      return Insert(key, value);
    }

    // Preform an insertion at dest.
    dest->state = Entry::kFull;
    dest->value = value;
    ++size_;

    return std::make_pair(true, dest);
  }
  // Returns (true, entry) on success, and (false, null) when key is not found.
  std::pair<bool, const Entry *> Delete(Key key) {
    auto hash = hasher_(key) % capacity_;

    // Try deleting up to capacity times.
    for (size_t i = 0; i < capacity_; ++i) {
      auto &entry = table_[hash];
      if (entry.state == Entry::kOpen) return std::make_pair(false, nullptr);
      if (entry.state == Entry::kFull && pred_(key, get_key_(entry.value))) {
        entry.state = Entry::kDeleted;
        --size_;
        return std::make_pair(true, &entry);
      }

      // Quadratic Probing.
      hash = i + i + 1;
      hash %= capacity_;
    }

    return std::make_pair(false, nullptr);
  }

  // Invokes the passed function on each entry in the table.
  // If the function returns true, the loop continues.
  // If the function returns false, the loop breaks.
  // The return value is true iff the function returned true for all entries.
  bool ForEach(const std::function<bool(const Entry *)> &f) const {
    bool okay = true;
    for (const Entry *e = table_.get(); e != table_.get() + capacity_; ++e) {
      okay = f(e);
      if (!okay) break;
    }

    return okay;
  }

  // Helper function for getting the index of an entry.
  size_t GetIndex(const Entry *entry) const {
    return entry - table_.get();
  }

 private:
  // Functor instances.
  KeyPred pred_;
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
