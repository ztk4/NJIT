#include "../bit_stream_3570.h"

#include <cstring>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace mp1 {
namespace {
// Test fixture for testing output bit streams.
class OutputBitStreamTest : public ::testing::Test {
 protected:
  ~OutputBitStreamTest() override = default;

  static constexpr char kTestByte = 'a';
  static constexpr char kTestString[] = "Testing";

  std::unique_ptr<OutputBitStream> MakeObs() {
    return std::unique_ptr<OutputBitStream>(new OutputBitStream(&os_));
  }

  // Default stream for use in most tests.
  std::ostringstream os_;
};

constexpr char OutputBitStreamTest::kTestByte;
constexpr char OutputBitStreamTest::kTestString[];

TEST_F(OutputBitStreamTest, OkWhenConstructedFromEmptyStream) {
  EXPECT_TRUE(MakeObs()->Ok());
}

TEST_F(OutputBitStreamTest, OkWhenConstructedFromOffsetStream) {
  os_ << "testval";
  EXPECT_TRUE(MakeObs()->Ok());
}

TEST_F(OutputBitStreamTest, PutsAlignedNumberOfBitsIndividually) {
  {
    auto obs = MakeObs();

    int mask = 0x01;
    for (int i = 0; i < 8; ++i) {
      obs->Put(kTestByte & mask);
      mask <<= 1;
    }
  }

  EXPECT_EQ(os_.str(), std::string(1, kTestByte));
}

TEST_F(OutputBitStreamTest, PutsAlignedNumebrOfBitsByVector) {
  {
    std::vector<bool> bits;

    int mask = 0x01;
    for (int i = 0; i < 8; ++i) {
      bits.push_back(kTestByte & mask);
      mask <<= 1;
    }

    auto obs = MakeObs();
    obs->Put(bits);
  }

  EXPECT_EQ(os_.str(), std::string(1, kTestByte));
}

TEST_F(OutputBitStreamTest, AlignedPackPutsString) {
  {
    auto obs = MakeObs();
    obs->AlignedPack(kTestString, strlen(kTestString));  // Without \0.
  }

  EXPECT_EQ(os_.str(), kTestString);
}

TEST_F(OutputBitStreamTest, PackInsertsCharWhenAligned) {
  {
    auto obs = MakeObs();
    obs->Pack(kTestByte);
  }

  EXPECT_EQ(os_.str(), std::string(1, kTestByte));
}

TEST_F(OutputBitStreamTest, PartialBitStateIsFlushed) {
  {
    auto obs = MakeObs();
    obs->Pack('I');
    obs->Put({true, false, true});
  }

  EXPECT_EQ(os_.str(), "I\x05");
}

TEST_F(OutputBitStreamTest, AlignOnBoundaryIsNoOp) {
  {
    auto obs = MakeObs();
    
    for (Align a : {Align::kOne, Align::kTwo, Align::kFour, Align::kEight}) {
      obs->Align(Align::kOne);
      EXPECT_TRUE(os_.str().empty()) << "Alignment to "
                                     << static_cast<int>(a)
                                     << " byte boundary resulted in output.";
    }
  }

  EXPECT_TRUE(os_.str().empty()) << "Destruction of stream resulted in output";
}

TEST_F(OutputBitStreamTest, AlignOneWorks) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true});
    obs->Align(Align::kOne);
  }

  EXPECT_EQ(os_.str(), "\x05");
}

TEST_F(OutputBitStreamTest, AlignTwoWorks) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true});
    obs->Align(Align::kTwo);
  }

  EXPECT_EQ(os_.str(), "\x05" + std::string(1, '\0'));
}

TEST_F(OutputBitStreamTest, AlignFourWorks) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true});
    obs->Align(Align::kFour);
  }

  EXPECT_EQ(os_.str(), "\x05" + std::string(3, '\0'));
}

TEST_F(OutputBitStreamTest, AlignEightWorks) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true});
    obs->Align(Align::kEight);
  }

  EXPECT_EQ(os_.str(), "\x05" + std::string(7, '\0'));
}

TEST_F(OutputBitStreamTest, AlignWithNonZeroPadWorks) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true});
    obs->Align(Align::kEight, 0xAB);
  }

  EXPECT_EQ(os_.str(), "\xAD" + std::string(7, '\xAB'));
}

TEST_F(OutputBitStreamTest, AlignedPackWorksWhenNotAlreadyAlgined) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true, true});
    obs->AlignedPack("\xDE\xAD\xBE\xEF", 4);
  }

  // 0xDEADBEEF shifted right 4 bits, padded with 0xD (0b1101) on the lsb side,
  // and 0x0 on the msb side (align call).
  EXPECT_EQ(os_.str(), "\xED\xDD\xEA\xFB\x0E");
}

TEST_F(OutputBitStreamTest, FlushSucceedsWhenAlignedToByteBoundary) {
  auto obs = MakeObs();
  obs->Put({true, false, true});
  obs->Align(Align::kOne);

  EXPECT_TRUE(obs->Flush());
  EXPECT_EQ(os_.str(), "\x05");
}

TEST_F(OutputBitStreamTest, FlushFailsWhenNotAlignedToByteBoundary) {
  auto obs = MakeObs();
  obs->Put({true, false, true});

  EXPECT_FALSE(obs->Flush());
  EXPECT_EQ(os_.str(), "");
}
}  // anonymous namespace
}  // namespace mp1
