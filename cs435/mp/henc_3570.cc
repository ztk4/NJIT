// Zachary Kaplan [3570]
// CS435-001
// MP 1
// 11/26/17

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "bit_stream_3570.h"
#include "prefix_cache_3570.h"

using mp1::PrefixCache;
using mp1::OutputBitStream;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::string;
using std::unordered_map;
using std::vector;

// ENCODING FORMAT:
// Four Byte Header: ZKHE (Zachary Kaplan Huffman Encoded).
// Serialized Prefix Codes: See prefix_cache_3570.cc for details.
// Huffman Encodig of the File: Sequential prefix codes for each byte of the
//                              original file.
// NOTE: An empty file is encoded to an empty file, this of course omits the
//       header and serialized prefix codes as well as the encoding section.

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " path/to/file\n";
    return -1;
  }

  string filename = argv[1];
  string huffname = filename + ".huf";
  {
    ifstream input(filename);
    ofstream output(huffname);

    if (!input) {
      cerr << "Unable to open input file " << filename << '\n';
      return -2;
    }
    if (!output) {
      cerr << "Unable to open output file " << huffname << '\n';
      return -3;
    }

    unordered_map<char, uint64_t> freq;
    while (true) {
      char c = input.get();
      if (!input) break;
      ++freq[c];
    }

    // We'll consider an empty file to be a special case, and allow our encoding
    // to be just an empty file as well.
    if (freq.size() != 0) {
      if (freq.size() == 1) {
        // Can't build a hufftree with a single character (formally).
        // As a practical workaround, we'll add a dummy character with frequency
        // 0 so that both the real and dummy characters get codes of length 1.
        char dummy = (freq.begin()->first + 1) & 0xFF;
        freq[dummy] = 0;
      }

      auto cache = PrefixCache::Make(
          vector<std::pair<char, uint64_t>>(freq.begin(), freq.end()));
      OutputBitStream obs(&output);

      // Write header to file.
      obs.AlignedPack("ZKHE", 4);
      // Serialize codes to file.
      cache.Serialize(&obs);
      // Encode the original file.
      input.clear();   // Clear flags on ifstream.
      input.seekg(0);  // Reset to beginning of file.
      while (true) {
        char c = input.get();
        if (!input) break;
        cache.WritePrefix(c, &obs);
      }
    }
  }

  // Delete the input file.
  if (remove(filename.c_str()) != 0) {
    cerr << "Unable to remove original file " << filename << '\n';
    return -4;
  }

  return 0;
}
