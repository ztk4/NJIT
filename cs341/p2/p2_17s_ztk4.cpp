// Zachary Trent Kaplan
// UCID: ztk4

#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <stack>
#include <string>

using namespace std;

/// Deterministic PDA implementation.
/// 
/// Does not allow for eplsilon transitions on the input string.
/// No restrictions on eplsilon transitions for the stack.
class PDA {
 public: 
  /// Character constant to represent epsilon in transitions.
  static const char kEpsilon;
  /// String constant to represent crashes.
  static const string kCrashState;

  /// A single directed transition between two states of the PDA
  struct Transition {
    Transition(char input, char pop, char push,
        const string &resultant_descriptor)
      : input(input),
        pop(pop),
        push(push),
        resultant_descriptor(resultant_descriptor) {}

    /// Input character read.
    char input;
    /// Character popped from stack.
    char pop;
    /// Character pushed to stack.
    char push;
    /// Descriptor of resulting state.
    string resultant_descriptor;
  };

  /// Typedef that defines a transition function for a single state.
  ///
  /// @param input the current character from the input string.
  /// @param stk the character at the top of the PDA's stack.
  /// 
  /// @returns the resulting transition taken.
  typedef function<Transition(char input, char stk)>
    TransitionFunction;

  /// Constructs a PDA that outputs verbosely to out, and starts in the state
  /// with the specified descriptor. To hide verbose output, set the stream 
  /// reference to a mock stream.
  ///
  /// @param start_descriptor string descriptor for the start state.
  /// @param out reference to the desired output stream.
  explicit PDA(const string &start_descriptor, ostream &out = cout)
    : start_descriptor_(start_descriptor),
      out_(out) {}
  // Disallow Copy & Assign
  PDA(const PDA &) = delete;
  PDA &operator=(const PDA &) = delete;

  /// Adds a state described uniquely by its descriptor to the PDA. If the
  /// given descriptor is already in use by the PDA, the addition will fail.
  ///
  /// @param descriptor a unique string descriptor for this state.
  /// @param delta the transition function that describes all transitions out
  ///              of this state.
  /// @param accepting whether or not this state is an accepting state.
  ///
  /// @returns true if the addition was successful, false otherwise.
  bool AddState(const string &descriptor, const TransitionFunction &delta,
      bool accepting = false) {
    // Inserts the state created from delta and accepting if key does not
    // already exist in the map.
    auto ret = states_.emplace(descriptor, State(delta, accepting));
    return ret.second;  // True if insert was successful, false otherwise.
  }

  /// Processes an input string and either accepts or rejects it. Resets the
  /// machine to the initial state before starting.
  /// 
  /// @param input_str the input string to process.
  /// 
  /// @returns true if the string was accepted, false otherwise.
  bool Process(const string &input_str);

 private:
  /// Private class representing a State in the PDA
  class State {
   public:
    /// Constructs a State based on its transition function and whether or not 
    /// its accepting.
    ///
    /// @param delta this states transition function.
    /// @param accapting whether or not this state is an accepting state.
    State(const TransitionFunction &delta, bool accepting)
      : delta_(delta), accepting_(accepting) {}

    /// Calls down to the transition function, and returns back the transition.
    ///
    /// @param input the current input character
    /// @param stk the character currently on top of the stack
    ///
    /// @returns a transition object describing the transition taken.
    Transition GetTransition(char input, char stk) const {
      return delta_(input, stk);
    }

    /// Used to determine if the state is accepting.
    ///
    /// @returns true if the state is accepting, false otherwise.
    bool IsAccepting() const {
      return accepting_;
    }

   private:
    /// This state's transition function.
    TransitionFunction delta_;
    /// Whether or not this state is an accepting state.
    bool accepting_;
  };

  /// The descriptor for the start state.
  string start_descriptor_;
  /// A mapping from state descriptors to their respective state objects.
  map<string, State> states_;
  /// The ostream to print verbose output to.
  ostream &out_;
};

// Static PDA members
const char PDA::kEpsilon = '\0';
const string PDA::kCrashState = "Crash";

// Printing Functions
ostream &operator<<(ostream &o, const PDA::Transition &t) {
  // Unicode Constants for Printing.
  const char *kEpsilonString = "\u03B5";
  const char *kRightArrowString = "\u2192";

  o << t.input << ", ";  // Input Character.

  // Pop Character or Epsilon.
  if (t.pop == PDA::kEpsilon)
    o << kEpsilonString;
  else
    o << t.pop;

  o << ' ' << kRightArrowString << ' ';  // Right Arrow and a Spaces.

  // Push Character or Epsilon.
  if (t.push == PDA::kEpsilon)
    o << kEpsilonString;
  else
    o << t.push;

  return o << endl << "Resultant State:\t" << t.resultant_descriptor;
}

// Non Inline Members of PDA.
bool PDA::Process(const string &input_str) {
  // Collective state of PDA while processing.
  stack<char> stk;
  auto curr_input_it = input_str.cbegin();
  auto curr_state_it = states_.find(start_descriptor_);
  
  out_ << "Start State:\t\t" << start_descriptor_ << endl;

  // While input is not expended, and the current state exists.
  while (curr_input_it != input_str.cend() &&
      curr_state_it != states_.end()) {
    // Get descriptor of next state. This also increments the input iterator.
    auto transition = curr_state_it->second.GetTransition(*curr_input_it++, 
        stk.empty() ? kEpsilon : stk.top());  // Only deref top if it exists
    // Manipulate the stack.
    if (transition.pop != kEpsilon)
      stk.pop();
    if (transition.push != kEpsilon)
      stk.push(transition.push);

    out_ << "Transition Taken:\t" << transition << endl;
    curr_state_it = states_.find(transition.resultant_descriptor);
  }

  // Invalid or Crash State.
  if (curr_state_it == states_.end()) {
    out_ << "The PDA has crashed on the input string." << endl;
    return false;
  }

  // Valid ending state.
  return curr_state_it->second.IsAccepting();
}


int main(int argc, char **argv) {
  // Construct a PDA with starting state q1
  PDA pda("q1");
  
  // The following state transition are equivalent to the edges shown on the pda
  // drawing included with the source code. Each if statement in the lambdas
  // acts as a filter for a given edge. The final return statement causes a
  // crash in the case that no other edges were matched from a given state.

  // Add state q1 to the PDA.
  pda.AddState("q1",
      [](char input, char stk) {
        if (input == '$') {
          return PDA::Transition(input, PDA::kEpsilon, '$', "q2");
        }

        return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon,
            PDA::kCrashState);
      });
  // Add state q2 to the PDA.
  pda.AddState("q2",
      [](char input, char stk) {
        if (input == '(') {
          return PDA::Transition(input, PDA::kEpsilon, '(', "q2");
        }

        if (isdigit(input)) {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q3");
        }

        if (isalpha(input) || input == '_') {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q4");
        }

        return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon,
            PDA::kCrashState);
      });
  // Add state q3 to the PDA.
  pda.AddState("q3",
      [](char input, char stk) {
        if (input == '$' && stk == '$') {
          return PDA::Transition(input, stk, PDA::kEpsilon, "q6");
        }

        if (input == ')' && stk == '(') {
          return PDA::Transition(input, stk, PDA::kEpsilon, "q5");
        }

        if (input == '+' || input == '-' || input == '*' || input == '/') {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q2");
        }

        if (isdigit(input)) {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q3");
        }

        return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon,
            PDA::kCrashState);
      });
  // Add state q4 to the PDA.
  pda.AddState("q4",
      [](char input, char stk) {
        if (input == '$' && stk == '$') {
          return PDA::Transition(input, stk, PDA::kEpsilon, "q6");
        }

        if (input == ')' && stk == '(') {
          return PDA::Transition(input, stk, PDA::kEpsilon, "q5");
        }

        if (input == '+' || input == '-' || input == '*' || input == '/') {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q2");
        }

        if (isalnum(input) || input == '_') {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q4");
        }

        return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon,
            PDA::kCrashState);
      });
  // Add state q5 to the PDA.
  pda.AddState("q5",
      [](char input, char stk) {
        if (input == '$' && stk == '$') {
          return PDA::Transition(input, stk, PDA::kEpsilon, "q6");
        }

        if (input == ')' && stk == '(') {
          return PDA::Transition(input, stk, PDA::kEpsilon, "q5");
        }

        if (input == '+' || input == '-' || input == '*' || input == '/') {
          return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon, "q2");
        }

        return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon,
            PDA::kCrashState);
      });
  // Add state q6 to the PDA.
  pda.AddState("q6",
      [](char input, char stk) {
        return PDA::Transition(input, PDA::kEpsilon, PDA::kEpsilon,
            PDA::kCrashState);
      }, true);  // Accepting State.

  // Main Loop.
  while (true) {
    cout << "Would you like to enter a string? (y/n): ";
    char resp;
    cin >> resp;
    if (resp == 'n')
      break;

    cout << "Enter your input string: ";
    string input_str;
    cin >> input_str;

    cout << "Input String: " << input_str << endl;

    // Redirects verbose output to cout by default.
    bool accepted = pda.Process(input_str);

    cout << "The input string was " << (accepted ? "ACCEPTED" : "REJECTED")
      << '.' << endl << endl;
  }

  return 0;
}
