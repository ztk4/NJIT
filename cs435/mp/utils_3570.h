// Zachary Kaplan [3570]
// CS435-001
// ALL
// PREEXISTING

// NOTE TO GRADER:
//   This is a utility file I wrote a while ago with some macros I like.
//   Does not directly pertain to the project, but these macros may pop up
//   throughout my code.

#ifndef UTIL_UTILS_H_
#define UTIL_UTILS_H_

#include <memory>

#include "logging_3570.h"

// Assert like helper macros CHECK and DCHECK.
// Allows streamed messages, eg CHECK(i == 0) << "i must be 0";
#define CHECK(cond) (::util::internal::CheckLogger(static_cast<bool>((cond)), \
                                                   #cond, __FILE__, __LINE__, \
                                                   __PRETTY_FUNCTION__))

#ifdef NDEBUG
#define DCHECK(cond) (::util::internal::NullLogger())
#else
#define DCHECK(cond) CHECK(cond)
#endif  // NDEBUG

// Checks ptr is not null, and returns it's value.
// Useful for asserting a non-null pointer, and then dereferencing it in one
// statement.
#define NOTNULL(ptr) \
  (CHECK(ptr) << "must not be null", (ptr))

namespace util {
// Names defined within ::util::internal are only to be called from this file
// and it's implementation.
namespace internal {
// Logger for the CHECK macros. References implementations in logging_3570.h
class CheckLogger {
 public:
  CheckLogger(bool cond, const char *const expr, const char *const filename,
      int file_num, const char *const pretty_func) : cond_(cond) {
    if (!cond_) {
      fatal_stream_logger_.reset(
          new FatalStreamLogger(FATAL, filename, file_num));
      (*fatal_stream_logger_) << "Check failed in " << pretty_func << ": "
                              << "'" << expr << "' ";
    }
  }

  template <typename T>
  CheckLogger &operator<<(const T &t) {
    if (!cond_) {
      (*fatal_stream_logger_) << t;
    }
    return *this;
  }

 private:
  const bool cond_;
  std::unique_ptr<FatalStreamLogger> fatal_stream_logger_;
};
}  // namespace internal
}  // namespace util

#endif  // UTIL_UTILS_H_
