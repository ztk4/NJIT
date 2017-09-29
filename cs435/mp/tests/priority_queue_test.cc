#include "../priority_queue_3570.h"

#include <algorithm>
#include <functional>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::UnorderedElementsAreArray;

namespace mp1 {
namespace {
class PriorityQueueTest : public ::testing::Test {
 protected:
  static constexpr int kSize = 5;
  static constexpr int kArr[kSize] = {42, 17, 19, -5, 20};
  static constexpr int const *kBegin = kArr;
  static constexpr int const *kEnd = kArr + kSize;

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
constexpr int const *PriorityQueueTest::kBegin;
constexpr int const *PriorityQueueTest::kEnd;

TEST_F(PriorityQueueTest, DefaultConstructsAsEmpty) {
  PriorityQueue<int> pq;

  EXPECT_TRUE(pq.empty());
  EXPECT_EQ(pq.size(), 0);
}

TEST_F(PriorityQueueTest, RangeConstructsToRange) {
  PriorityQueue<int> pq(kBegin, kEnd);

  EXPECT_FALSE(pq.empty());
  EXPECT_EQ(pq.size(), kSize);
  
  auto vec = ToVec(&pq);

  EXPECT_THAT(vec, UnorderedElementsAreArray(kArr, kSize));
}

TEST_F(PriorityQueueTest, EmptyRangeConstructsAsEmpty) {
  PriorityQueue<int> pq(kBegin, kBegin);

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

// In the following tests, Inversions refer to inversions of the binary-heap
// invariant in the input initializer lists. 
TEST_F(PriorityQueueTest, HandlesInputWithNoInversions) {
  PriorityQueue<int> pq1 = {1, 17, 42};
  PriorityQueue<int> pq2 = {1, 42, 17};

  EXPECT_THAT(ToVec(&pq1), ElementsAre(1, 17, 42));
  EXPECT_THAT(ToVec(&pq2), ElementsAre(1, 17, 42));
}

TEST_F(PriorityQueueTest, HandlesInputWithLeftInversion) {
  PriorityQueue<int> pq = {17, 1, 42};

  EXPECT_THAT(ToVec(&pq), ElementsAre(1, 17, 42));
}

TEST_F(PriorityQueueTest, HandlesInputWithRightInversion) {
  PriorityQueue<int> pq = {17, 42, 1};

  EXPECT_THAT(ToVec(&pq), ElementsAre(1, 17, 42));
}

TEST_F(PriorityQueueTest, HandlesInputWithBothInversions) {
  PriorityQueue<int> pq1 = {42, 17, 1};
  PriorityQueue<int> pq2 = {42, 1, 17};

  EXPECT_THAT(ToVec(&pq1), ElementsAre(1, 17, 42));
  EXPECT_THAT(ToVec(&pq2), ElementsAre(1, 17, 42));
}

TEST_F(PriorityQueueTest, ElementsAreSortedByDefaultComparator) {
  PriorityQueue<int> pq(kBegin, kEnd);

  std::vector<int> sorted(kBegin, kEnd);
  std::sort(sorted.begin(), sorted.end());

  auto elts = ToVec(&pq);

  EXPECT_THAT(elts, ElementsAreArray(sorted));
}

TEST_F(PriorityQueueTest, ElementsAreSortedBySpecifiedComparator) {
  PriorityQueue<int, std::greater<int>> pq(kBegin, kEnd);

  std::vector<int> sorted(kBegin, kEnd);
  std::sort(sorted.begin(), sorted.end(), std::greater<int>());

  auto elts = ToVec(&pq);

  EXPECT_THAT(elts, ElementsAreArray(sorted));
}

TEST_F(PriorityQueueTest, PopRemovesBestElement) {
  PriorityQueue<int> pq(kBegin, kEnd);
  int size = pq.size();

  EXPECT_EQ(pq.Pop(), *std::min_element(kBegin, kEnd));
  EXPECT_EQ(pq.size(), size - 1);
}

TEST_F(PriorityQueueTest, PeekReturnsNextElementToBePopped) {
  PriorityQueue<int> pq(kBegin, kEnd);

  int peeked = pq.Peek();

  EXPECT_EQ(peeked, pq.Pop());
}

TEST_F(PriorityQueueTest, PushIncreasesSize) {
  PriorityQueue<int> empty;
  PriorityQueue<int> pq(kBegin, kEnd);

  int size = pq.size();

  pq.Push(5);
  empty.Push(42);

  EXPECT_EQ(pq.size(), size + 1);
  EXPECT_EQ(empty.size(), 1);
}

TEST_F(PriorityQueueTest, PushMaintainsOrder) {
  PriorityQueue<int> pq(kBegin, kEnd);
  std::vector<int> sorted(kBegin, kEnd);

  for (int i : {12, 6, 23, 63}) {
    pq.Push(i);
    sorted.push_back(i);
  }

  std::sort(sorted.begin(), sorted.end());

  EXPECT_THAT(ToVec(&pq), ElementsAreArray(sorted));
}

TEST_F(PriorityQueueTest, PushPopMaintainsPreformsBothPushAndPop) {
  PriorityQueue<int> pq(kBegin, kEnd);
  std::vector<int> sorted(kBegin, kEnd);
  int size = pq.size();

  auto min = std::min_element(sorted.begin(), sorted.end());

  // This test does not test the edge case of when the new element is the min.
  int new_val = *min + 1;
  EXPECT_EQ(*min, pq.PushPop(new_val));
  *min = new_val;  // Replace min in sorted as well.

  EXPECT_EQ(pq.size(), size);  // Size didn't decrease or increase.

  std::sort(sorted.begin(), sorted.end());
  EXPECT_EQ(ToVec(&pq), sorted);
}

TEST_F(PriorityQueueTest, PushPopHandlesEmptyQueue) {
  PriorityQueue<int> empty;
  
  EXPECT_EQ(42, empty.PushPop(42));
  EXPECT_TRUE(empty.empty());
}

TEST_F(PriorityQueueTest, PushPopReturnsPassedValueIfBest) {
  PriorityQueue<int> pq(kBegin, kEnd);

  int min = *std::min_element(kBegin, kEnd);
  int new_val = min - 1;

  EXPECT_EQ(new_val, pq.PushPop(new_val));
}
}  // anonymous namespace
}  // namespace mp1
