#include "timeout.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "gtest/gtest.h"

using namespace std;

// NOTE: I should spend time to mock out a clock and then make a sleep method
// for threads based on that clock so that my unit tests don't actually have
// to sleep. BUT that is a lot of work compared to the goal of this class...

namespace util {
namespace {
// Timeout Test Fixture
class TimeoutTest : public ::testing::Test {
 protected:
  static const chrono::milliseconds kPeriod;

  TimeoutTest() : timeout_(new SimpleTimeout(kPeriod)) {}

  unique_ptr<Timeout> timeout_;
};

const chrono::milliseconds TimeoutTest::kPeriod = chrono::milliseconds(1);

// In the following tests, [](){} is just a dummy callback 

TEST_F(TimeoutTest, IsNotActiveBeforeStart) {
  EXPECT_FALSE(timeout_->IsActive());
}

TEST_F(TimeoutTest, IsActiveAfterStart) {
  timeout_->Start([](){});
  
  EXPECT_TRUE(timeout_->IsActive());
}

TEST_F(TimeoutTest, IsNotActiveAfterCancel) {
  timeout_->Start([](){});
  timeout_->Cancel();

  EXPECT_FALSE(timeout_->IsActive());
}

TEST_F(TimeoutTest, StartsWhileInactive) {
  EXPECT_TRUE(timeout_->Start([](){}));
}

TEST_F(TimeoutTest, DoesNotStartWhileActive) {
  timeout_->Start([](){});
  EXPECT_FALSE(timeout_->Start([](){}));
}

TEST_F(TimeoutTest, DoesNotCancelWhileInactive) {
  EXPECT_FALSE(timeout_->Cancel());
}

TEST_F(TimeoutTest, CancelsWhileActive) {
  timeout_->Start([](){});
  EXPECT_TRUE(timeout_->Cancel());
}

TEST_F(TimeoutTest, TimeoutOccursAfterTimeoutPeriodWhenActive) {
  atomic<bool> called(false);
  timeout_->Start([&called]() { called = true; });

  EXPECT_FALSE(called);

  // Wait for twice timeout (not precise)
  this_thread::sleep_for(kPeriod * 2);

  EXPECT_TRUE(called);
}

TEST_F(TimeoutTest, TimeoutDoesNotOccurWhenCanceled) {
  atomic<bool> called(false);
  timeout_->Start([&called]() { called = true; });

  EXPECT_FALSE(called);
  timeout_->Cancel();

  // Wait for twice timeout (not precise)
  this_thread::sleep_for(kPeriod * 2);

  EXPECT_FALSE(called);
}
}  // namespace util
}  // anonymous namespace
