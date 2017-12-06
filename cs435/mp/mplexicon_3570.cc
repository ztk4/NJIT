// Zachary Kaplan [3570] CS435-001 MP
// MP 2
// 11/30/17

#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <memory>
#include <string>

#include "lexicon_3570.h"

using mp2::Lexicon;
using std::cerr;
using std::cout;
using std::ifstream;
using std::numeric_limits;
using std::streamsize;
using std::string;
using std::unique_ptr;

void ProcessBatchCommands(ifstream *commands) {
  unique_ptr<Lexicon> lex;
  while (*commands) {
    int op;
    (*commands) >> op;
    if (!*commands) break;
    if (!lex && op != 14) {
      cerr << "Cannot preform operation " << op
           << " before creating a lexicon\n";
      // Ignore rest of invalid line.
      commands->ignore(numeric_limits<streamsize>::max(), '\n');
      continue;
    }

    switch (op) {
      case 10: {  // Insert
        string str;
        (*commands) >> str;
        ssize_t loc = lex->Insert(str.c_str());
        if (loc < 0) cerr << "Failed to insert " << str << '\n';
        break;
      }
      case 11: {  // Delete
        string str;
        (*commands) >> str;
        ssize_t loc = lex->Delete(str.c_str());
        if (loc >= 0) cout << str << "\t deleted from slot " << loc << '\n';
        else cout << str << "\t not deleted because not found\n";
        break;
      }
      case 12: {  // Search
        string str;
        (*commands) >> str;
        ssize_t loc = lex->Search(str.c_str());
        if (loc >= 0) cout << str << "\t found at slot " << loc << '\n';
        else cout << str << "\t not found\n";
        break;
      }
      case 13: {  // Print
        lex->Print(&cout);
        cout << '\n';
        break;
      }
      case 14: {  // Create
        size_t capacity;
        (*commands) >> capacity;
        lex.reset(new Lexicon(capacity));
        break;
      }
      default: {
        cerr << "Unrecognized operation " << op << '\n';
        break;
      }
    }

    // Make sure to be at next line for next command.
    commands->ignore(numeric_limits<streamsize>::max(), '\n');
  }
}

int main(int argc, char **argv) {
  if (argc != 2) {
    cerr << "Usage: " << *argv << " command.txt\n";
    return -1;
  }

  ifstream commands(argv[1]);
  if (!commands) {
    cerr << "Failed to open file '" << argv[1] << "'\n";
    return -2;
  }

  ProcessBatchCommands(&commands);

  return 0;
}
