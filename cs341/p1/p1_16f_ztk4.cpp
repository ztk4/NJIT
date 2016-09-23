// Zachary Kaplan
// UCID: ztk4

#include <functional>
#include <iostream>
#include <string>
#include <vector>

using std::cin;
using std::cout;
using std::endl;
using std::function;
using std::ostream;
using std::string;
using std::vector;

class State {
 public:
  State() : out_(&cout) {}
  State(int number, ostream *out,
        function<const State &(const char *)> transition,
        bool accepted = false)
    : number_(number),
      out_(out),
      transition_(transition),
      accepted_(accepted) {}
  State(const State &state)
    : number_(state.number_),
      out_(state.out_),
      transition_(state.transition_),
      accepted_(state.accepted_) {}
  
  const State &operator=(const State &state) {
    number_ = state.number_;
    out_ = state.out_;
    transition_ = state.transition_;
    accepted_ = state.accepted_;
  }

  // Process a string through the DFA.
  //
  // @param str Pointer to char at current position in the input string.
  //            Must be a NULL terminated string.
  // @returns True if the string is in the language, and false otherwise.
  bool Process(const char *str) const {
    *out_ << *this << endl;                       // Print State Name
    
    if (*str) {
      *out_ << *str << ' ';                       // Print current character
      return transition_(str).Process(str + 1);   // Transition and process
    }

    return accepted_;                             // Done
  }

  friend ostream &operator<<(ostream &o, const State &);

 private:
  // State Number (stored for printing)
  int number_;
  // Stream (for printing as well)
  ostream *out_;
  // Transition function for this state
  function<const State &(const char *)> transition_;
  // True if an accepted state, false otherwise
  bool accepted_;
};

ostream &operator<<(ostream &o, const State &state) {
  return o << 'q' << state.number_;
}

int main(int argc, char **argv) {
  // State Declarations
  vector<State> q(1);  // Not using the first position to simplify numbering.
  q.reserve(17);
  
  // Emplace new State with number == index in q
  // Printing is to cout
  // Lambdas taking a char ptr and returning a const State & contain a switch
  // statement for determining the next state. They capture q by REFERENCE to
  // select states.
  // Optional final true value will set state as accepted.
  q.emplace_back(1, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'w':
          return q[2];
        case '.':
          return q[13];
        default:
          return q[5];
      }
    });
  q.emplace_back(2, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'w':
          return q[3];
        case '.':
          return q[7];
        default:
          return q[5];
      }
    });
  q.emplace_back(3, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'w':
          return q[4];
        case '.':
          return q[7];
        default:
          return q[5];
      }
    });
  q.emplace_back(4, &cout, [&q] (const char *str) -> const State & {
      if (*str == '.')
        return q[6];
      return q[5];
    });
  q.emplace_back(5, &cout, [&q] (const char *str) -> const State & {
      if (*str == '.')
        return q[7];
      return q[5];
    });
  q.emplace_back(6, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'c':
          return q[14];
        case '.':
          return q[13];
        default:
          return q[5];
      }
    });
  q.emplace_back(7, &cout, [&q] (const char *str) -> const State & {
      if (*str == 'c')
        return q[8];
      return q[13];
    });
  q.emplace_back(8, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'a':
          return q[10];
        case 'o':
          return q[9];
        default:
          return q[13];
      }
    });
  q.emplace_back(9, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'm':
          return q[10];
        case '.':
          return q[11];
        default:
          return q[13];
      }
    });
  q.emplace_back(10, &cout, [&q] (const char *str) -> const State & {
      return q[13];
    }, true);  // Accepted State
  q.emplace_back(11, &cout, [&q] (const char *str) -> const State & {
      if (*str == 'c')
        return q[12];
      return q[13];
    });
  q.emplace_back(12, &cout, [&q] (const char *str) -> const State & {
      if (*str == 'a')
        return q[10];
      return q[13];
    });
  q.emplace_back(13, &cout, [&q] (const char *str) -> const State & {
      return q[13];
    });  // Trap State
  q.emplace_back(14, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'a':
          return q[16];
        case 'o':
          return q[15];
        case '.':
          return q[7];
        default:
          return q[5];
      }
    });
  q.emplace_back(15, &cout, [&q] (const char *str) -> const State & {
      switch (*str) {
        case 'm':
          return q[16];
        case '.':
          return q[7];
        default:
          return q[5];
      }
    });
  q.emplace_back(16, &cout, [&q] (const char *str) -> const State & {
      if (*str == '.')
        return q[7];
      return q[5];
    }, true);  // Accepted State

  // Input Processing
  char c;

  cout << "Would you like to enter a string? ";
  cin >> c;
  while (c == 'y') {
    string str;

    cout << "Enter your string: ";
    cin >> str;

    if (q[1].Process(str.c_str()))
      cout << "Accepted";
    else
      cout << "Denied";
    cout << endl;

    cout << "Would you like to enter another string? ";
    cin >> c;
  }

  return 0;
}
