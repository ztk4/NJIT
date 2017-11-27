#include "../prefix_cache_3570.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "../bit_stream_3570.h"

using ::testing::ElementsAre;

namespace mp1 {
namespace {
class PrefixCacheTest : public ::testing::Test {
 protected:
  ~PrefixCacheTest() override = default;

  void ResetStreams(const std::string &contents = "") {
    ss_.str(contents);
    ibs_.reset(new InputBitStream(&ss_));
    obs_.reset(new OutputBitStream(&ss_));
  }

  char GetChar(PrefixCache *cache, std::vector<bool> prefix) {
    ResetStreams();
    obs_->Put(prefix);
    obs_->Align(Align::kOne);
    obs_->Flush();

    return cache->GetChar(ibs_.get());
  }

  std::vector<bool> GetPrefix(PrefixCache *cache, char c) {
    ResetStreams();
    int num_bits = cache->WritePrefix(c, obs_.get());
    obs_->Align(Align::kOne);
    obs_->Flush();

    return ibs_->Get(num_bits);
  }

  std::stringstream ss_;
  std::unique_ptr<InputBitStream> ibs_;
  std::unique_ptr<OutputBitStream> obs_;
};

TEST_F(PrefixCacheTest, 2CharsWithLog2Frequencies) {
  auto cache = PrefixCache::Make({{'a', 5}, {'b', 10}});

  EXPECT_EQ(GetChar(&cache, {0}), 'a');
  EXPECT_EQ(GetChar(&cache, {1}), 'b');

  EXPECT_THAT(GetPrefix(&cache, 'a'), ElementsAre(0));
  EXPECT_THAT(GetPrefix(&cache, 'b'), ElementsAre(1));
}

TEST_F(PrefixCacheTest, 3CharsWithLog2Frequencies) {
  auto cache = PrefixCache::Make({{'a', 6}, {'b', 12}, {'c', 24}});

  EXPECT_EQ(GetChar(&cache, {0, 0}), 'a');
  EXPECT_EQ(GetChar(&cache, {0, 1}), 'b');
  EXPECT_EQ(GetChar(&cache, {1}), 'c');

  EXPECT_THAT(GetPrefix(&cache, 'a'), ElementsAre(0, 0));
  EXPECT_THAT(GetPrefix(&cache, 'b'), ElementsAre(0, 1));
  EXPECT_THAT(GetPrefix(&cache, 'c'), ElementsAre(1));
}

TEST_F(PrefixCacheTest, 4CharsWithLog2Frequencies) {
  auto cache = PrefixCache::Make({{'a', 7}, {'b', 14}, {'c', 28}, {'d', 56}});

  EXPECT_EQ(GetChar(&cache, {0, 0, 0}), 'a');
  EXPECT_EQ(GetChar(&cache, {0, 0, 1}), 'b');
  EXPECT_EQ(GetChar(&cache, {0, 1}), 'c');
  EXPECT_EQ(GetChar(&cache, {1}), 'd');

  EXPECT_THAT(GetPrefix(&cache, 'a'), ElementsAre(0, 0, 0));
  EXPECT_THAT(GetPrefix(&cache, 'b'), ElementsAre(0, 0, 1));
  EXPECT_THAT(GetPrefix(&cache, 'c'), ElementsAre(0, 1));
  EXPECT_THAT(GetPrefix(&cache, 'd'), ElementsAre(1));
}

TEST_F(PrefixCacheTest, 4CharsUniformDistribution) {
  auto cache = PrefixCache::Make({{'a', 42}, {'b', 42}, {'c', 42}, {'d', 42}});
  
  // The exact code each character gets here is not clear.
  // But they all should be of length 2 exactly.
  for (char c = 'a'; c <= 'd'; ++c) {
    EXPECT_EQ(GetPrefix(&cache, c).size(), 2) << "character '" << c << "'";
  }
}

TEST_F(PrefixCacheTest, 8CharsApproximatelyUniformDistribution) {
  auto cache = PrefixCache::Make({{'a', 55}, {'b', 53}, {'c', 56}, {'d', 57},
                                  {'e', 54}, {'f', 55}, {'g', 58}, {'h', 52}});

  // Same as above test case, except size 3.
  for (char c = 'a'; c <= 'h'; ++c) {
    EXPECT_EQ(GetPrefix(&cache, c).size(), 3) << "character '" << c << "'";
  }
}

TEST_F(PrefixCacheTest, DeserializationOfSerializedCacheIsValid) {
  auto cache = PrefixCache::Make({{'a', 7}, {'b', 14}, {'c', 28}, {'d', 56}});

  std::stringstream serial;
  {
    OutputBitStream obs(&serial);
    cache.Serialize(&obs);
  }

  InputBitStream ibs(&serial);
  auto cache_from_serial = PrefixCache::Deserialize(&ibs);

  // Assert that the properties of the original cache hold.
  EXPECT_EQ(GetChar(&cache_from_serial, {0, 0, 0}), 'a');
  EXPECT_EQ(GetChar(&cache_from_serial, {0, 0, 1}), 'b');
  EXPECT_EQ(GetChar(&cache_from_serial, {0, 1}), 'c');
  EXPECT_EQ(GetChar(&cache_from_serial, {1}), 'd');

  EXPECT_THAT(GetPrefix(&cache_from_serial, 'a'), ElementsAre(0, 0, 0));
  EXPECT_THAT(GetPrefix(&cache_from_serial, 'b'), ElementsAre(0, 0, 1));
  EXPECT_THAT(GetPrefix(&cache_from_serial, 'c'), ElementsAre(0, 1));
  EXPECT_THAT(GetPrefix(&cache_from_serial, 'd'), ElementsAre(1));
}
}  // anonymous namespace
}  // namespace mp1
