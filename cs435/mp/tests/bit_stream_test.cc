#include "../bit_stream_3570.h"

#include <cstring>
#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::StrEq;

namespace mp1 {
namespace {
constexpr char kTestByte = 'a';
// NOTE: This must be at least eight characters.
constexpr char kTestString[] = "This is a test string!";

// Test fixture for testing OutputBitStreams.
class OutputBitStreamTest : public ::testing::Test {
 protected:
  ~OutputBitStreamTest() override = default;

  std::unique_ptr<OutputBitStream> MakeObs() {
    return std::unique_ptr<OutputBitStream>(new OutputBitStream(&os_));
  }

  // Default stream for use in most tests.
  std::ostringstream os_;
};

TEST_F(OutputBitStreamTest, OkWhenConstructedFromEmptyStream) {
  EXPECT_TRUE(MakeObs()->Ok());
}

TEST_F(OutputBitStreamTest, OkWhenConstructedFromOffsetStream) {
  os_ << kTestString;
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
      obs->Align(a);
      EXPECT_TRUE(os_.str().empty()) << "Alignment to "
                                     << static_cast<int>(a)
                                     << " byte boundary resulted in output";
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

TEST_F(OutputBitStreamTest, PackWorksWhenNotAlreadyAligned) {
  {
    auto obs = MakeObs();
    obs->Put({true, false, true, true});
    obs->Pack(static_cast<int32_t>(0xDEADBEEF));
  }

  EXPECT_EQ(os_.str(), "\xFD\xEE\xDB\xEA\x0D");
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

// Test fixture for InputBitStreams.
class InputBitStreamTest : public ::testing::Test {
 protected:
  ~InputBitStreamTest() override = default;

  std::unique_ptr<InputBitStream> MakeIbs() {
    return std::unique_ptr<InputBitStream>(new InputBitStream(&is_));
  }

  // Default string stream to use for constructing InputBitStreams.
  std::istringstream is_;
};

TEST_F(InputBitStreamTest, OkWhenConstructedFromEmptyStream) {
  auto ibs = MakeIbs();
  EXPECT_TRUE(ibs->Ok());
}

TEST_F(InputBitStreamTest, OkWhenConstructedFromStreamWithContents) {
  is_.str(kTestString);

  auto ibs = MakeIbs();

  EXPECT_TRUE(ibs->Ok());
}

TEST_F(InputBitStreamTest, GetsAlignedNumberOfBitsIndividually) {
  is_.str(std::string(1, kTestByte));

  auto ibs = MakeIbs();

  int mask = 0x01;
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(ibs->Get(), static_cast<bool>(kTestByte & mask)) 
      << "Mismatch between stream value and expected value at bit #" << i;
    mask <<= 1;
  }
}

TEST_F(InputBitStreamTest, GetsAlignedNumberOfBitsByVector) {
  is_.str(std::string(1, kTestByte));

  auto ibs = MakeIbs();

  auto bits = ibs->Get(8);

  int mask = 0x01;
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(bits[i], static_cast<bool>(kTestByte & mask))
      << "Mismatch between streamed bits and expected value at bit #" << i;
    mask <<= 1;
  }
}

TEST_F(InputBitStreamTest, AlignedUnpackGetsString) {
  is_.str(kTestString);

  auto ibs = MakeIbs();

  char buf[sizeof(kTestString)] = {0};
  ibs->AlignedUnpack(buf, strlen(kTestString));

  EXPECT_THAT(buf, StrEq(kTestString));
}

TEST_F(InputBitStreamTest, UnpackGetsCharWhenAligned) {
  is_.str(std::string(1, kTestByte));

  auto ibs = MakeIbs();

  EXPECT_EQ(ibs->Unpack<char>(), kTestByte);
}

TEST_F(InputBitStreamTest, AlignOnBoundaryIsNoOp) {
  is_.str(kTestString);

  auto ibs = MakeIbs();

  for (Align a : {Align::kOne, Align::kTwo, Align::kFour, Align::kEight}) {
    ibs->Align(a);
    EXPECT_EQ(is_.tellg(), 0) << "Alignment to " << static_cast<int>(a)
                              << " byte boundary modified the stream";
  }
}

TEST_F(InputBitStreamTest, AlignOneWorks) {
  is_.str(kTestString);

  auto ibs = MakeIbs();
  ibs->Get();  // Pull off a bit to knock off the boundary.
  ibs->Align(Align::kOne);

  EXPECT_EQ(is_.tellg(), 1);
}

TEST_F(InputBitStreamTest, AlignTwoWorks) {
  is_.str(kTestString);

  auto ibs = MakeIbs();
  ibs->Get();  // Pull off a bit to knock off the boundary.
  ibs->Align(Align::kTwo);

  EXPECT_EQ(is_.tellg(), 2);
}

TEST_F(InputBitStreamTest, AlignFourWorks) {
  is_.str(kTestString);

  auto ibs = MakeIbs();
  ibs->Get();  // Pull off a bit to knock off the boundary.
  ibs->Align(Align::kFour);

  EXPECT_EQ(is_.tellg(), 4);
}

TEST_F(InputBitStreamTest, AlignEightWorks) {
  is_.str(kTestString);

  auto ibs = MakeIbs();
  ibs->Get();  // Pull off a bit to knock off the boundary.
  ibs->Align(Align::kEight);

  EXPECT_EQ(is_.tellg(), 8);
}

TEST_F(InputBitStreamTest, AlignedUnpackWorksWhenNotAlreadyAligned) {
  is_.str("\xDE\xAD\xBE\xEF");

  auto ibs = MakeIbs();
  
  // Take out the first 4 bits (0xE).
  int res = 0;
  for (int i = 0; i < 4; ++i) {
    res |= (ibs->Get() << i);
  }
  EXPECT_EQ(res, 0xE);

  char buf[5] = {0};
  ibs->AlignedUnpack(buf, 4);

  EXPECT_THAT(buf, StrEq("\xDD\xEA\xFB\x0E"));
}

TEST_F(InputBitStreamTest, UnpackWorksWhenNotAlreadyAligned) {
  is_.str("\xDE\xAD\xBE\xEF");

  auto ibs = MakeIbs();

  // Take out the first 4 bits (0xE).
  int res = 0;
  for (int i = 0; i < 4; ++i) {
    res |= (ibs->Get() << i);
  }
  EXPECT_EQ(res, 0xE);

  auto val = ibs->Unpack<int32_t>();
  EXPECT_EQ(val, 0xEFBEADD);
}

class StreamIntegrationTest : public ::testing::Test {
 protected:
  StreamIntegrationTest() : ibs_(&ss_), obs_(&ss_) {}
  ~StreamIntegrationTest() override = default;

  static const std::vector<bool> kBits;
  static constexpr char kTestChar = 0x42;
  static constexpr int32_t kTestInt = 0xDEADBEEF;
  static constexpr float kTestFloat = 1.23e4;
  static constexpr double kTestDouble = 5.67e89;


  void AlignAndFlush(Align a = Align::kOne) {
    obs_.Align(a);
    obs_.Flush();
  }

  std::stringstream ss_;
  InputBitStream ibs_;
  OutputBitStream obs_;
};

const std::vector<bool> StreamIntegrationTest::kBits =
    {true, false, true, true};
constexpr char StreamIntegrationTest::kTestChar;
constexpr int32_t StreamIntegrationTest::kTestInt;
constexpr float StreamIntegrationTest::kTestFloat;
constexpr double StreamIntegrationTest::kTestDouble;

TEST_F(StreamIntegrationTest, IndividualBits) {
  for (bool bit : kBits)
    obs_.Put(bit);

  AlignAndFlush();

  for (size_t i = 0; i < kBits.size(); ++i)
    EXPECT_EQ(ibs_.Get(), kBits[i]) << "Bits differ at index " << i;
}

TEST_F(StreamIntegrationTest, VectorOfBits) {
  obs_.Put(kBits);

  AlignAndFlush();

  EXPECT_EQ(ibs_.Get(kBits.size()), kBits);
}

TEST_F(StreamIntegrationTest, PackAndUnpack) {
  obs_.Pack(kTestChar);
  obs_.Pack(kTestInt);
  obs_.Pack(kTestFloat);
  obs_.Pack(kTestDouble);

  AlignAndFlush();

  EXPECT_EQ(ibs_.Unpack<char>(), kTestChar);
  EXPECT_EQ(ibs_.Unpack<int32_t>(), kTestInt);
  EXPECT_EQ(ibs_.Unpack<float>(), kTestFloat);
  EXPECT_EQ(ibs_.Unpack<double>(), kTestDouble);
}

TEST_F(StreamIntegrationTest, AlignedPackAndUnpack) {
  obs_.AlignedPack(kTestString, strlen(kTestString));

  AlignAndFlush();

  char buf[sizeof(kTestString)] = {0};
  ibs_.AlignedUnpack(buf, strlen(kTestString));

  EXPECT_THAT(buf, StrEq(kTestString));
}

TEST_F(StreamIntegrationTest, AllAtOnce) {
  obs_.Put(true);
  obs_.Pack(kTestChar);
  obs_.Pack(kTestInt);
  obs_.Pack(kTestFloat);
  obs_.Pack(kTestDouble);
  obs_.Put(false);
  obs_.AlignedPack(kTestString, strlen(kTestString));
  obs_.Put(kBits);

  AlignAndFlush();

  EXPECT_TRUE(ibs_.Get());
  EXPECT_EQ(ibs_.Unpack<char>(), kTestChar);
  EXPECT_EQ(ibs_.Unpack<int32_t>(), kTestInt);
  EXPECT_EQ(ibs_.Unpack<float>(), kTestFloat);
  EXPECT_EQ(ibs_.Unpack<double>(), kTestDouble);
  EXPECT_FALSE(ibs_.Get());

  char buf[sizeof(kTestString)] = {0};
  ibs_.AlignedUnpack(buf, strlen(kTestString));

  EXPECT_THAT(buf, StrEq(kTestString));
  EXPECT_EQ(ibs_.Get(kBits.size()), kBits);
}
}  // anonymous namespace
}  // namespace mp1
