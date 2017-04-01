#include "distance_vector_table.h"

#include <cstdint>
#include <vector>
#include <utility>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

using namespace std;
using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::SizeIs;

namespace router {
namespace {
class DistanceVectorTableTest : public ::testing::Test {
 protected:
  static const vector<pair<uint16_t, int16_t>> kStartingEdges;
  static const pair<uint16_t, int16_t> kNewEdge;
  static const pair<uint16_t, int16_t> kBetterEdge;
  static const pair<uint16_t, int16_t> kWorseEdge;
  static const uint16_t kSourceId;

  DistanceVectorTableTest() : dvt_(kSourceId) {}

  void FillTable() {
    for (const auto &edge : kStartingEdges)
      dvt_.AddEdge(edge.first, edge.second);
  }

  DistanceVectorTable dvt_;
};

const vector<pair<uint16_t, int16_t>> DistanceVectorTableTest::kStartingEdges =
  { {25, 10}, {4, 6}, {11, 15} };
const pair<uint16_t, int16_t> DistanceVectorTableTest::kNewEdge = {17, 19};
const pair<uint16_t, int16_t> DistanceVectorTableTest::kBetterEdge = {25, 2};
const pair<uint16_t, int16_t> DistanceVectorTableTest::kWorseEdge = {25, 5};
const uint16_t DistanceVectorTableTest::kSourceId = 42;

TEST_F(DistanceVectorTableTest, TableInitiallyContainsOneEdgeOfWeightZero) {
  EXPECT_THAT(dvt_.Table(), ElementsAre(make_pair(kSourceId, 0)));
}

TEST_F(DistanceVectorTableTest, TableContainsEdgeAfterSuccessfulAddition) {
  EXPECT_TRUE(dvt_.AddEdge(12, 24));
  
  EXPECT_THAT(dvt_.Table(), Contains(make_pair(12, 24)));
}

TEST_F(DistanceVectorTableTest, TableDoesNotAddEdgeAfterUnsuccessfulAddition) {
  dvt_.AddEdge(12, 24);
  EXPECT_FALSE(dvt_.AddEdge(12, 42));

  EXPECT_THAT(dvt_.Table(), SizeIs(2));  // Self-edge and one good edge
}

TEST_F(DistanceVectorTableTest, TableDoesNotUpdateWithUnknownSourceId) {
  EXPECT_FALSE(dvt_.UpdateTable(217, {kNewEdge}));

  EXPECT_THAT(dvt_.Table(), SizeIs(1));
}

TEST_F(DistanceVectorTableTest, TableUpdatesWithEdgeToUnknownDestination) {
  FillTable();
  EXPECT_TRUE(dvt_.UpdateTable(4, {kNewEdge}));

  EXPECT_THAT(dvt_.Table(),
      Contains(make_pair(kNewEdge.first, kNewEdge.second + 6)));
}

TEST_F(DistanceVectorTableTest, TableUpdatesWithBetterEdgeToKnownDestination) {
  FillTable();
  EXPECT_TRUE(dvt_.UpdateTable(4, {kBetterEdge}));

  EXPECT_THAT(dvt_.Table(),
      Contains(make_pair(kBetterEdge.first, kBetterEdge.second + 6)));
}

TEST_F(DistanceVectorTableTest,
    TableDoesNotUpdateWithWorseEdgeToKnownDestination) {
  FillTable();
  EXPECT_FALSE(dvt_.UpdateTable(4, {kWorseEdge}));

  EXPECT_THAT(dvt_.Table(), Contains(make_pair(kWorseEdge.first, 10)));
}
}  // anonymous namespace
}  // namespace router
