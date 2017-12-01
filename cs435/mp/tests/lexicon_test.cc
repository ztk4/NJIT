#include "../lexicon_3570.h"

#include "gtest/gtest.h"

namespace mp2 {
namespace {
class LexiconTest : public ::testing::Test {
 protected:
  ~LexiconTest() override = default;

  static constexpr size_t kCap = 15;
};

constexpr size_t LexiconTest::kCap;

TEST_F(LexiconTest, InsertIncreasesSizeByOne) {
  Lexicon lex(kCap);
  
  EXPECT_GE(lex.Insert("testing"), 0);
  EXPECT_EQ(lex.Size(), 1);
}

TEST_F(LexiconTest, DoubleInsertFails) {
  Lexicon lex(kCap);

  lex.Insert("testing");
  EXPECT_LT(lex.Insert("testing"), 0);
}

TEST_F(LexiconTest, SearchFindsStringAfterInsert) {
  Lexicon lex(kCap);

  auto placed = lex.Insert("testing");

  auto found = lex.Search("testing");
  EXPECT_GE(found, 0);

  EXPECT_EQ(placed, found);
}

TEST_F(LexiconTest, SearchDoesNotFindAbsentString) {
  Lexicon lex(kCap);

  lex.Insert("testing");

  EXPECT_LT(lex.Search("different"), 0);
}

TEST_F(LexiconTest, DeleteDescreaesSizeByOne) {
  Lexicon lex(kCap);

  auto placed = lex.Insert("testing");

  auto removed = lex.Delete("testing");
  EXPECT_GE(removed, 0);

  EXPECT_EQ(placed, removed);
  EXPECT_EQ(lex.Size(), 0);
}

TEST_F(LexiconTest, DeleteFailsOnAbsentString) {
  Lexicon lex(kCap);

  lex.Insert("testing");

  EXPECT_LT(lex.Delete("different"), 0);
  EXPECT_EQ(lex.Size(), 1);
}

TEST_F(LexiconTest, InsertSucceedsWhenArenaSizeIsSmall) {
  Lexicon lex(kCap);
  lex.SetArenaSize(5);

  EXPECT_GE(lex.Insert("testing"), 0);
}

TEST_F(LexiconTest, InsertSucceedsWhenArenaSizeIsVerySmall) {
  Lexicon lex(kCap);
  lex.SetArenaSize(1);

  EXPECT_GE(lex.Insert("VeryVeryVeryVeryVeryVeryVeryVeryVeryLongString"), 0);
}
}  // anonymous namespace
}  // namespace mp2
