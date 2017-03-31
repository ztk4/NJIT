#include "thread_pool.h"

#include <map>
#include <set>
#include <vector>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using testing::Each;
using namespace std;

namespace util {
namespace {
class ThreadPoolTest : public ::testing::Test {
 protected:
  ThreadPoolTest() : indexed_run_(10), pool_(4), count_(0) {
    for (int key : map_keys_) {
      mapped_run_[key] = false;
    }
  }

  map<int, atomic<bool>> mapped_run_;
  set<int> map_keys_ = {1, 25, 42};
  vector<atomic<bool>> indexed_run_;
  ThreadPool pool_;
  atomic<int> count_;
};

TEST_F(ThreadPoolTest, MapExecutesWithEachArgumentInRange) {
  pool_.Map([this](int key) {
      mapped_run_[key] = true;
    }, map_keys_.begin(), map_keys_.end());

  for (const auto &entry : mapped_run_)
    EXPECT_EQ(entry.second, true);
}

TEST_F(ThreadPoolTest, MapAsyncExecutesWithEachArgumentInRangeAfterJoinAll) {
  pool_.MapAsync([this](int key) {
      mapped_run_[key] = true;
    }, map_keys_.begin(), map_keys_.end());

  pool_.JoinAll();

  for (const auto &entry : mapped_run_)
    EXPECT_EQ(entry.second, true);
}

TEST_F(ThreadPoolTest, ExecutesTasksWithThreadIdTheRightNumberOfTimes) {
  pool_.Execute([this](int id) {
      indexed_run_[id] = true;
    }, 10);

  for (bool val : indexed_run_)
    EXPECT_TRUE(val);
}

TEST_F(ThreadPoolTest, ExecutesTasksAsyncWithThreadIdTheRightNumberOfTimes) {
  pool_.ExecuteAsync([this](int id) {
      indexed_run_[id] = true;
    }, 10);

  pool_.JoinAll();

  for (bool val : indexed_run_)
    EXPECT_TRUE(val);
}

TEST_F(ThreadPoolTest, ExecutesProperNumberOfTasksWithoutThreadId) {
  pool_.Execute([this]() {
      ++count_;
    }, 15);

  EXPECT_EQ(count_, 15);
}

TEST_F(ThreadPoolTest, ExecutesProperNumberOfTasksAsyncWithoutThreadId) {
  pool_.ExecuteAsync([this]() {
      ++count_;
    }, 15);

  pool_.JoinAll();

  EXPECT_EQ(count_, 15);
}
}  // anonymous namespace
}  // namespace util
