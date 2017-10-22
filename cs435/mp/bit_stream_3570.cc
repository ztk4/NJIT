#include "bit_stream_3570.h"

#include "utils_3570.h"

namespace mp1 {
namespace {
// Bit Helpers by Mask
// Set the bits specified by mask.
inline uint8_t MaskSet(uint8_t byte, uint8_t mask) {
  return byte | mask;
}
// Unset the bits specified by mask.
inline uint8_t MaskUnset(uint8_t byte, uint8_t mask) {
  return byte & ~mask;
}
// Put the bits specified by mask to the bits specified by val.
inline uint8_t MaskPut(uint8_t val, uint8_t byte, uint8_t mask) {
  return MaskSet(MaskUnset(byte, mask), val & mask);
}

// Mask Helpers
// Gets a mask for just the speicifed bit (0-indexed).
inline uint8_t BitMask(uint8_t bit) {
  DCHECK(bit < 8) << "Index " << bit << " larger than bits in byte (8).";
  return 1 << bit;
}
// Gets a mask for this bit and all bits more significant than this bit.
inline uint8_t MsbMask(uint8_t bit) {
  DCHECK(bit < 8) << "Index " << bit << " larger than bits in byte (8).";
  return 0xFF << bit;
}
// Gets a mask for this bit and all bits less significant than this bit.
inline uint8_t LsbMask(uint8_t bit) {
  DCHECK(bit < 8) << "Index " << bit << " larger than bits in byte (8).";
  return 0xFF >> (7 - bit);
}

// Single Bit Helpers
// Sets the given bit of byte (0-indexed).
inline uint8_t BitSet(uint8_t byte, uint8_t bit) {
  return MaskSet(byte, BitMask(bit));
}
// Unsets the given bit of byte (0-indexed).
inline uint8_t BitUnset(uint8_t byte, uint8_t bit) {
  return MaskUnset(byte, BitMask(bit));
}
// Puts the given boolean at the given bit of byte (0-indexed).
inline uint8_t BitPut(bool b, uint8_t byte, uint8_t bit) {
  return b ? BitSet(byte, bit) : BitUnset(byte, bit);
}
}  // anonymous namespace

// OutputBitStream Implementation
OutputBitStream::OutputBitStream(std::ostream *os) : os_(*NOTNULL(os)) {}

bool OutputBitStream::Flush() {
  if (bits_) {  // Not aligned to a byte boundary.
    return false;
  }

  os_.flush();
  return Ok();
}

void OutputBitStream::Align(mp1::Align n, uint8_t pad) {
  if (bits_) {  // curr_ contains valid state.
    // Set all from bits on in curr_ to pad.
    curr_ = MaskPut(pad, curr_, MsbMask(bits_));
    // Put the character, and reset internal state.
    os_.put(curr_);
    curr_ = bits_ = 0;
  }

  auto boundary = static_cast<long long>(n);
  auto rem = os_.tellp() % boundary;
  if (rem) {  // Alignment is needed.
    // Write the padding character until we hit the byte boundary.
    do {
      os_.put(pad);
    } while (++rem < boundary);
  }
}

void OutputBitStream::AlignedPack(const char *mem, size_t len) {
  if (!bits_) {  // No current state.
    // Just stream the bytes.
    os_.write(mem, len);
    return;
  }

  // Bit index of where to split each byte of mem.
  // byte 0..split (inclusive) goes into the more significant of the current
  //               byte.
  // byte split+1..7 (inclusive) goes into the less significant part of the next
  //                 byte.
  uint8_t split = 7 - bits_;
  for (const char *byte = mem; byte < mem + len; ++byte) {
    // Lower and Upper part of current byte of mem (about split).
    auto lower = *byte & LsbMask(split);
    auto upper = *byte & MsbMask(split + 1);
    
    // Lower part of byte goes into upper part of curr_.
    curr_ |= (lower << bits_);
    os_.put(curr_);
    // Upper part of byte goes into lower part of next curr_.
    curr_ = (upper >> (split + 1));
  }
}

void OutputBitStream::Put(bool bit) {
  // Put the bit value into curr_ at the next available bit (indexed by bits).
  curr_ = BitPut(bit, curr_, bits_);
  // Increment bits, and push to stream if needed.
  if (++bits_ >= 8) {
    os_.put(curr_);
    bits_ = curr_ = 0;
  }
}

void OutputBitStream::Put(const std::vector<bool> &bits) {
  for (bool bit : bits) {
    Put(bit);
  }
}
}  // namespace mp1
