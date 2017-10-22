// Zachary Kaplan [3570]
// CS435-001
// MP 1
// 9/30/17

#ifndef MP1_BIT_STREAM_H_
#define MP1_BIT_STREAM_H_

#include <cstdint>
#include <iostream>
#include <vector>

namespace mp1 {
// Specifies byte alignment options.
enum class Align { 
  kOne   = 0x1,
  kTwo   = 0x2,
  kFour  = 0x4,
  kEight = 0x8,
};

class OutputBitStream {
 public:
  // Constructs an output bit stream from an ostream. Does not take ownership of
  // the stream, and will not destruct or close it.
  // NB: os may not be nullptr
  explicit OutputBitStream(std::ostream *os);
  // Disallow copy and assign
  OutputBitStream(const OutputBitStream &) = delete;
  OutputBitStream &operator=(const OutputBitStream &) = delete;

  // Aligns the stream, and then flushes it to ensure the internal stream gets
  // the entire state of this bitstream.
  ~OutputBitStream() { Align(mp1::Align::kOne); Flush(); }

  // Flushes the internal stream. 
  // NB: The bitstream must be at a byte boundary in order to flush. Will fail
  //     if there is a partial byte state. Call Align(Align::kOne) to ensure
  //     a success.
  bool Flush();

  // Aligns the stream to a n-byte boundry, if at that boundry this is a no-op.
  // Pads out to the boundary using the padding byte.
  // Returns true on success.
  void Align(mp1::Align n, uint8_t pad = 0x00);

  // Packs an arbitray value into the stream. The value will be packed as its
  // memory layout, so this should only be used on POD types.
  // NOTE: bool and vector<bool> are packed as their exact memory layout.
  //       Prefer to insert bools and vector<bool> via Put.
  template <typename T>
  void Pack(const T &t) {
    const char *mem = static_cast<const char *>(&t);
    AlignedPack(mem, sizeof(T));
  }
  // Packs arbitary memory pointed to by mem of length len in BYTES.
  void AlignedPack(const char *mem, size_t len);

  // Puts a single bit into the stream.
  void Put(bool bit);
  // Puts a vector of bools into the stream (each element as a single bit).
  void Put(const std::vector<bool> &bits);

  // Returns the state of the underlying stream.
  bool Ok() const { return static_cast<bool>(os_); }

 private:
  // Underlying stream.
  std::ostream &os_;
  // Current byte state.
  uint8_t curr_ = 0;
  // Number of bits in curr_ that have meaningful state (starting at LSB).
  // 0 indiciates that there is NO partial state in curr_.
  uint8_t bits_ = 0;
};

// Free OutputBitStream operator overloads.
// Default implementations for bool, vector<bool>, and arithmetic types.
inline OutputBitStream &operator<<(OutputBitStream &obs, bool bit) {
  obs.Put(bit);
  return obs;
}
inline OutputBitStream &operator<<(OutputBitStream &obs,
    const std::vector<bool> &bits) {
  obs.Put(bits);
  return obs;
}
template <typename Arithmetic,
          std::enable_if<std::is_arithmetic<Arithmetic>::value>* = nullptr>
inline OutputBitStream &operator<<(OutputBitStream &obs, Arithmetic a) {
  obs.Pack(a);
  return obs;
}

// TODO: Implement.
class InputBitStream {};
}  // namespace mp1

#endif  // MP1_BIT_STREAM_H_
