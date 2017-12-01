// Zachary Kaplan [3570]
// CS435-001
// MP2
// 11/30/17

#include "lexicon_3570.h"

#include <cstring>
#include <utility>

#include "logging_3570.h"

namespace mp2 {
Lexicon::Lexicon(size_t capacity)
  : table_(capacity, StringEq(), StringHasher(), GetKey(this)),
    arena_size_(capacity * 15),
    arena_(new char[arena_size_]),
    arena_base_(0) {
  // Initialize arena to spaces.
  memset(arena_.get(), ' ', arena_size_);
}

bool Lexicon::SetArenaSize(size_t arena_size) {
  if (arena_base_ > arena_size) return false;
  if (arena_size_ == arena_size) return true;

  LOG(DEBUG) << "Setting arena size to " << arena_size;

  // Allocate a new arena.
  std::unique_ptr<char[]> new_arena(new char[arena_size]);

  // Copy existing data to new arena (only nead to copy up to arena_base).
  memcpy(new_arena.get(), arena_.get(), arena_base_);

  // Move the new arena into this Lexicon, and change arena_size_.
  arena_ = std::move(new_arena);
  arena_size_ = arena_size;

  // Set the excess (if any) of the new arena to spaces.
  if (arena_size_ > arena_base_) {
    memset(arena_.get() + arena_base_, ' ', arena_size_ - arena_base_);
  }

  return true;
}

ssize_t Lexicon::Search(const char *str) const {
  auto res = table_.Search(str);
  return (res.first ? res.second : -1);
}

ssize_t Lexicon::Insert(const char *str) {
  bool success = table_.Insert(str, arena_base_);
  if (success) {
    // Check if the string will fit in the arena, and resize if needed.
    size_t len = strlen(str);
    if (len >= arena_size_ - arena_base_) {
      size_t new_size = arena_size_ * 2;
      // Just in case strings are really large.
      while (len >= new_size - arena_base_) new_size *= 2;
      bool resized = SetArenaSize(new_size);
      if (!resized) return -1;
    }

    // Insert the string into the arena, and update arena_base_.
    size_t old_base = arena_base_;
    strcpy(arena_.get() + arena_base_, str);
    arena_base_ += len;
    return old_base;
  }

  return -1;
}

ssize_t Lexicon::Delete(const char *str) {
  auto res = table_.Delete(str);
  if (res.first) {
    // Set this string's representation to all '*' in the arena.
    for (char *c = arena_.get() + res.second; *c; ++c) *c = '*';
    return res.second;
  }

  return -1;
}

// Regular string comparison.
bool Lexicon::StringEq::operator()(const char *str1, const char *str2) const {
  return !strcmp(str1, str2);
}

// Simple additive hash.
size_t Lexicon::StringHasher::operator()(const char *str) const {
  size_t hash = 0;

  for (const char *c = str; *c; ++c) {
    hash += *c;
  }

  return hash;
}

// Fetches a key string from the arena based on it's index.
const char *Lexicon::GetKey::operator()(size_t idx) const {
  return lexicon->arena_.get() + idx;
}
}  // namespace mp2
