#include "socket.h"

#include <memory>

#include "gtest/gtest.h"

using namespace std;

namespace util {
namespace {
class InAddressTest : public ::testing::Test {
 protected:
  static const in_addr_t kAddress;
  static const in_port_t kPort;

  InAddressTest() : addr_(new InSocket::InAddress(kAddress, kPort)) {}

  const unique_ptr<Socket::Address> addr_;
};

const in_addr_t InAddressTest::kAddress = 0x42424242;
const in_port_t InAddressTest::kPort = 0x4242;

TEST_F(InAddressTest, AddressValuesAreProperlySet) {
  auto *addr_p = reinterpret_cast<const sockaddr_in *>(addr_->Value());

  EXPECT_EQ(addr_p->sin_family, AF_INET);
  EXPECT_EQ(addr_p->sin_port, kPort);
  EXPECT_EQ(addr_p->sin_addr.s_addr, kAddress);
}

TEST_F(InAddressTest, AddressLengthIsCorrect) {
  struct sockaddr_in dummy;

  EXPECT_EQ(addr_->Length(), sizeof(dummy));
}

TEST_F(InAddressTest, CopyConstructionWorks) {
  InSocket::InAddress addr(
      *static_cast<const InSocket::InAddress *>(addr_.get()));

  auto *addr_p = reinterpret_cast<const sockaddr_in *>(addr_->Value());

  EXPECT_EQ(addr_p->sin_family, AF_INET);
  EXPECT_EQ(addr_p->sin_port, kPort);
  EXPECT_EQ(addr_p->sin_addr.s_addr, kAddress);
}

TEST_F(InAddressTest, CopyAssignmentWorks) {
  // Avoids implicit invocation of Copy Constructor
  InSocket::InAddress addr(0, 0);
  addr = *static_cast<const InSocket::InAddress *>(addr_.get());

  auto *addr_p = reinterpret_cast<const sockaddr_in *>(addr_->Value());

  EXPECT_EQ(addr_p->sin_family, AF_INET);
  EXPECT_EQ(addr_p->sin_port, kPort);
  EXPECT_EQ(addr_p->sin_addr.s_addr, kAddress);
}

TEST(InSocketTest, IsOpenAfterConstruction) {
  unique_ptr<Socket> socket(new InSocket(InSocket::UDP));

  EXPECT_TRUE(socket->IsOpen());
}
}  // anonymous namespace
}  // namespace util
