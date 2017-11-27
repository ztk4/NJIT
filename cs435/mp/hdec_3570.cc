// Zachary Kaplan [3570]
// CS435-001
// MP 1
// 11/26/17

#include <fstream>
#include <iostream>
#include <string>

#include "bit_stream_3570.h"
#include "logging_3570.h"
#include "prefix_cache_3570.h"

using mp1::PrefixCache;
using mp1::InputBitStream;
using std::cerr;
using std::ifstream;
using std::ofstream;
using std::string;

// See henc_3570.cc for encoding format.

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " path/to/file.huf\n";
    return -1;
  }

  string huffname = argv[1];
  if (huffname.substr(huffname.size() - 4) != ".huf") {
    cerr << "Invalid file extension, must end in .huf\n";
    return -2;
  }
  string filename = huffname.substr(0, huffname.size() - 4);
  {
    ifstream input(huffname);
    ofstream output(filename);

    if (!input) {
      cerr << "Unable to open input file " << huffname << '\n';
      return -3;
    }
    if (!output) {
      cerr << "Unable to open output file " << filename << '\n';
      return -4;
    }

    InputBitStream ibs(&input);
    char first = ibs.Unpack<char>();
    if (ibs.Ok()) {  // File not empty.
      if (first != 'Z' || ibs.Unpack<char>() != 'K' ||
          ibs.Unpack<char>() != 'H' || ibs.Unpack<char>() != 'E') {
        cerr << "Invalid header, cannot read input file " << huffname << '\n';
        return -5;
      }

      auto cache = PrefixCache::Deserialize(&ibs);
      while (true) {
        char c = cache.GetChar(&ibs);
        if (!ibs.Ok()) break;
        output.put(c);
      }
    }
  }

  if (remove(huffname.c_str()) != 0) {
    cerr << "Unable to remove original file " << huffname << '\n';
    return -6;
  }

  return 0;
}
