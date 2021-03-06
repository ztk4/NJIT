#ifndef UTIL_LOGGING_H_
#define UTIL_LOGGING_H_

#include <ios>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>

// Macro for logging. Provides thread safe logging (message is not broken up)
// and multiple levels of logging. DEBUG level is optimized out in production
// builds.
// NOTE: Names must be padded to length 8
#define INFO (::util::internal::StreamLogger(&::std::cout, "INFO    "))
#define WARN (::util::internal::StreamLogger(&::std::cerr, "WARNING "))
#define ERR  (::util::internal::StreamLogger(&::std::cerr, "ERROR   "))
#ifdef NDEBUG
  #define DEBUG (::util::internal::NullLogger())
#else
  #define DEBUG (::util::internal::StreamLogger(&::std::cout, "DEBUG   "))
#endif  // NDEBUG

namespace util {
/// An internal namespace for util. Nothing inside of this namespace should be
/// referenced anywhere but this file.
namespace internal {
/// A logger using ostreams.
class StreamLogger {
 public:
  StreamLogger(std::ostream *stream, const char *prefix) : stream_(stream) {
    buffer_ << '[' << prefix
      << "@0x" << std::hex << std::this_thread::get_id()
      << "] " << std::dec;
  }
  ~StreamLogger() {
    buffer_ << std::endl;
    (*stream_) << buffer_.str();
  }

  template <typename T>
  StreamLogger &operator<<(const T &t) {
    buffer_ << t;
    return *this;
  }

  // For manipulators, pass onto sstream
  StreamLogger &operator<<(std::ios_base &(*manip)(std::ios_base &)) {
    buffer_ << manip;
    return *this;
  }
  StreamLogger &operator<<(std::ios &(*manip)(std::ios &)) {
    buffer_ << manip;
    return *this;
  }
  StreamLogger &operator<<(std::ostream &(*manip)(std::ostream &)) {
    buffer_ << manip;
    return *this;
  }

 private:
  std::ostream *stream_;
  std::stringstream buffer_;
};

/// Used to ignore log messages. Should be optimized out.
class NullLogger {
 public:
  template <typename T>
  NullLogger &operator<<(const T &) { return *this; }
};
}  // namespace internal
}  // namespace util

#endif  // UTIL_LOGGING_H_
