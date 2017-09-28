#include "../priority_queue_3570.h"

#include <algorithm>
#include <functional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAreArray;
using ::testing::UnorderedElementsAreArray;

namespace mp1 {
namespace {
class PriorityQueueTest : public ::testing::Test {
 protected:
  static constexpr int kSize = 5;
  static constexpr int kArr[kSize] = {42, 17, 19, -5, 20};

  PriorityQueueTest() = default;
  ~PriorityQueueTest() override = default;

  template <typename T, typename U>
  std::vector<T> ToVec(PriorityQueue<T, U> *pq) {
    std::vector<T> out;
    while (!pq->empty())
      out.push_back(pq->Pop());

    return out;
  }

};

constexpr int PriorityQueueTest::kSize;
constexpr int PriorityQueueTest::kArr[];

TEST_F(PriorityQueueTest, DefaultConstructsAsEmpty) {
  PriorityQueue<int> pq;

  EXPECT_TRUE(pq.empty());
  EXPECT_EQ(pq.size(), 0);
}

TEST_F(PriorityQueueTest, RangeConstructsToRange) {
  PriorityQueue<int> pq(kArr, kArr + kSize);

  EXPECT_FALSE(pq.empty());
  EXPECT_EQ(pq.size(), kSize);
  
  auto vec = ToVec(&pq);

  EXPECT_THAT(vec, UnorderedElementsAreArray(kArr, kSize));
}

TEST_F(PriorityQueueTest, EmptyRangeConstructsAsEmpty) {
  PriorityQueue<int> pq(kArr, kArr);

  EXPECT_TRUE(pq.empty());
  EXPECT_EQ(pq.size(), 0);
}

TEST_F(PriorityQueueTest, InitializerListConstructsToList) {
  // Deliberately constructed with an initializer list containing all the same
  // elements as kArr to allow for easier checking.
  PriorityQueue<int> pq = {42, 17, 19, -5, 20};

  EXPECT_FALSE(pq.empty());
  EXPECT_EQ(pq.size(), kSize);
  
  auto vec = ToVec(&pq);

  EXPECT_THAT(vec, UnorderedElementsAreArray(kArr, kSize));
}

TEST_F(PriorityQueueTest, ElementsAreSortedByDefaultComparator) {
  PriorityQueue<int> pq(kArr, kArr + kSize);

  std::vector<int> sorted(kArr, kArr + kSize);
  std::sort(sorted.begin(), sorted.end());

  auto elts = ToVec(&pq);

  EXPECT_THAT(elts, ElementsAreArray(sorted));
}

TEST_F(PriorityQueueTest, ElementsAreSortedBySpecifiedComparator) {
  PriorityQueue<int, std::greater<int>> pq(kArr, kArr + kSize);

  std::vector<int> sorted(kArr, kArr + kSize);
  std::sort(sorted.begin(), sorted.end(), std::greater<int>());

  auto elts = ToVec(&pq);

  EXPECT_THAT(elts, ElementsAreArray(sorted));
}
}  // anonymous namespace
}  // namespace mp1
