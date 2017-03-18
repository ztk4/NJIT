#ifndef UTIL_MOCK_TIMEOUT_H_
#define UTIL_MOCK_TIMEOUT_H_

#include "timeout.h"

#include "gmock/gmock.h"

namespace util {
/// Mock Timeout. ONLY FOR TESTING.
class MockTimeout : public Timeout {
 public:
  MOCK_METHOD1(Start, bool(const std::function<void(void)> &callback));
  MOCK_METHOD0(Cancel, bool());
  MOCK_CONST_METHOD(IsActive, bool());
};
}  // namespace util

#endif  // UTIL_MOCK_TIMEOUT_H_
