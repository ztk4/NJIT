// Zachary Kaplan [3570] CS435-001 MP
// ALL
// PREEXISTING

// NOTE TO GRADER:
//   This is a utility file I wrote a while ago to help me with logging.
//   Does not directly pertain to the project, as I handle I/O for non-logging
//   purposes in other ways.

#ifndef UTIL_LOGGING_H_
#define UTIL_LOGGING_H_

#include <cstdlib>
#include <ios>
#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <thread>
#include <utility>

// Macro for logging. Provides thread safe logging (message is not broken up)
// and multiple levels of logging. DEBUG level is optimized out in production
// builds.
#define LOG(level) (typename ::util::internal::logger_type<(level)>::value( \
      (level), __FILE__, __LINE__))

// Log levels, usually passed to the LOG macro.
enum LogLevel {
  INFO,    // Regular informative log.
  WARN,    // Warning log.
  ERROR,   // Error log (serious, but recoverable).
  FATAL,   // Error log (serious and not recoverable; calls abort()).
  DEBUG,   // Used for debugging, is a NoOp when NDEBUG is defined.
  DFATAL,  // Acts like FATAL in debug mode, and ERROR when NDEBUG is defined.
};

namespace util {
// An internal namespace for util. Nothing inside of this namespace should be
// referenced anywhere outside of util's implementations.
namespace internal {
// A logger using ostreams.
class StreamLogger {
 public:
  StreamLogger(LogLevel level, const char *const filename, int line_num) {
    buffer_ << '[';
    
    switch (level) {
      case INFO:
        buffer_ << "INFO    ";
        stream_ = &std::cout;
        break;
      case WARN:
        buffer_ << "WARNING ";
        stream_ = &std::cerr;
        break;
      case ERROR:
      case FATAL:
      case DFATAL:
        buffer_ << "ERROR   ";
        stream_ = &std::cerr;
        break;
      case DEBUG:  // This is only ever called when NDEBUG is not defined.
        buffer_ << "DEBUG   ";
        stream_ = &std::cout;
        break;
    }

    buffer_ << filename << ":" << line_num << "] " << std::dec;
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

// A stream logger that aborts after printing.
class FatalStreamLogger {
 public:
  template <typename... Args>
  FatalStreamLogger(Args&&... args)
    : stream_logger_(new StreamLogger(std::forward<Args>(args)...)) {}
  ~FatalStreamLogger() {
    stream_logger_.reset();  // Destroy stream logger (flushes to stream).
    abort();
  }

  template <typename T>
  FatalStreamLogger &operator<<(const T &t) {
    (*stream_logger_) << t;
    return *this;
  }

 private:
  // Owned as a unique_ptr to allow early destruction.
  std::unique_ptr<StreamLogger> stream_logger_;
};

// Used to ignore log messages. Should be optimized out.
class NullLogger {
 public:
  // Null-op constructor allows uniform construction with other loggers.
  template <typename... Args>
  NullLogger(Args&&...) {}

  template <typename T>
  NullLogger &operator<<(const T &) { return *this; }
};

// Gets the logger type based on the log level (defaults to StreamLogger).
template <LogLevel level>
struct logger_type {
  using value = StreamLogger;
};
// Template specializations for some log levels.
template <>
struct logger_type<FATAL> {
  using value = FatalStreamLogger;
};
#ifdef NDEBUG
// DEBUG is NoOp in NDEBUG mode.
template <>
struct logger_type<DEBUG> {
  using value = NullLogger;
};
#else
// DFATAL is fatal in debug mode.
template<>
struct logger_type<DFATAL> {
  using value = FatalStreamLogger;
};
#endif  // NDEBUG
}  // namespace internal
}  // namespace util

#endif  // UTIL_LOGGING_H_
