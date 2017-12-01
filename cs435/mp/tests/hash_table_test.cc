#include "../hash_table_3570.h"

#include <functional>

#include "gtest/gtest.h"

namespace mp2 {
namespace {
class HashTableTest : public ::testing::Test {
 protected:
  ~HashTableTest() override = default;

  // Types for HashTable.
  using Key = int;
  struct Value {
    int key;
    int value;
  };
  // Bad hash, good for testing though.
  struct Hasher {
    explicit Hasher(const std::function<void()> &cb = []{}) : cb(cb) {}
    Hasher(const Hasher &) = default;
    Hasher &operator=(const Hasher &) = default;
    Hasher(Hasher &&) = default;
    Hasher &operator=(Hasher &&) = default;

    size_t operator()(int key) const {
      if (cb) cb();
      return key;
    }

    std::function<void()> cb;
  };
  struct GetKey {
    explicit GetKey(const std::function<void()> &cb = []{}) : cb(cb) {}
    GetKey(const GetKey &) = default;
    GetKey &operator=(const GetKey &) = default;
    GetKey(GetKey &&) = default;
    GetKey &operator=(GetKey &&) = default;

    const Key &operator()(const Value &value) const {
      if (cb) cb();
      return value.key;
    }

    std::function<void()> cb;
  };
  struct KeyPred {
    explicit KeyPred(const std::function<void()> &cb = []{}) : cb(cb) {}
    KeyPred(const KeyPred &) = default;
    KeyPred &operator=(const KeyPred &) = default;
    KeyPred(KeyPred &&) = default;
    KeyPred &operator=(KeyPred &&) = default;

    bool operator()(int k1, int k2) const { 
      if (cb) cb();
      return k1 == k2;
    }

    std::function<void()> cb;
  };
  using TestTable = HashTable<Key, Value, KeyPred, Hasher, GetKey>;

  // Useful constants.
  static constexpr size_t kCap = 15;
};

constexpr size_t HashTableTest::kCap;

TEST_F(HashTableTest, CapacityOnlyConstructor) {
  TestTable table(kCap);

  EXPECT_TRUE(table.Empty());
  EXPECT_FALSE(table.Full());
  
  EXPECT_EQ(table.Size(), 0);
  EXPECT_EQ(table.Capacity(), kCap);
}

TEST_F(HashTableTest, ConstructorWithFunctors) {
  bool pred_cb_called = false;
  bool hash_cb_called = false;
  bool key_cb_called = false;

  auto pred_cb = [&pred_cb_called] { pred_cb_called = true; };
  auto hash_cb = [&hash_cb_called] { hash_cb_called = true; };
  auto key_cb = [&key_cb_called] { key_cb_called = true; };

  TestTable table(kCap, KeyPred(pred_cb), Hasher(hash_cb), GetKey(key_cb));

  EXPECT_TRUE(table.Empty());
  EXPECT_FALSE(table.Full());

  EXPECT_EQ(table.Size(), 0);
  EXPECT_EQ(table.Capacity(), kCap);

  // Should invoke the hash.
  table.Insert(1, Value{1, 2});
  // Should invoke the key cb and the predicate cb.
  table.Search(1);

  EXPECT_TRUE(hash_cb_called);
  EXPECT_TRUE(key_cb_called);
  EXPECT_TRUE(pred_cb_called);
}

TEST_F(HashTableTest, FunctorsCarryOverAfterSetCapacity) {
  bool pred_cb_called = false;
  bool hash_cb_called = false;
  bool key_cb_called = false;

  auto pred_cb = [&pred_cb_called] { pred_cb_called = true; };
  auto hash_cb = [&hash_cb_called] { hash_cb_called = true; };
  auto key_cb = [&key_cb_called] { key_cb_called = true; };

  TestTable table(kCap, KeyPred(pred_cb), Hasher(hash_cb), GetKey(key_cb));

  table.SetCapacity(kCap * 2);

  table.Insert(1, Value{1, 2});
  table.Search(1);

  EXPECT_TRUE(hash_cb_called);
  EXPECT_TRUE(key_cb_called);
  EXPECT_TRUE(pred_cb_called);
}

TEST_F(HashTableTest, SetCapacityIncreasesTheCapacity) {
  TestTable table(kCap);

  table.SetCapacity(kCap * 5);

  EXPECT_EQ(table.Capacity(), kCap * 5);
}

TEST_F(HashTableTest, InsertIncreasesSize) {
  TestTable table(kCap);

  bool success = table.Insert(1, Value{1, 2});

  EXPECT_TRUE(success);
  EXPECT_EQ(table.Size(), 1);
}

TEST_F(HashTableTest, CannotDoubleInsertKey) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  EXPECT_FALSE(table.Insert(1, Value{1, 3}));
  EXPECT_EQ(table.Size(), 1);
}

TEST_F(HashTableTest, SearchFindsInsertedKey) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  auto res = table.Search(1);

  EXPECT_TRUE(res.first);
  EXPECT_EQ(res.second.key, 1);
  EXPECT_EQ(res.second.value, 2);
}

TEST_F(HashTableTest, SearchFailsOnMissingKey) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  auto res = table.Search(2);

  EXPECT_FALSE(res.first);
}

TEST_F(HashTableTest, SearchFailsAfterCollision) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  auto res = table.Search(1 + kCap);

  EXPECT_FALSE(res.first);
}

TEST_F(HashTableTest, InsertSucceedsAfterCollision) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  bool success = table.Insert(1 + kCap, Value{1 + kCap, 10});

  EXPECT_TRUE(success);
}

TEST_F(HashTableTest, SearchSucceedsAfterCollision) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});
  table.Insert(1 + kCap, Value{1 + kCap, 10});

  auto res = table.Search(1 + kCap);

  EXPECT_TRUE(res.first);
  EXPECT_EQ(res.second.key, 1 + kCap);
  EXPECT_EQ(res.second.value, 10);
}

TEST_F(HashTableTest, DeleteFailsOnMissingKey) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  auto res = table.Delete(5);

  EXPECT_FALSE(res.first);
  EXPECT_EQ(table.Size(), 1);
}

TEST_F(HashTableTest, DeleteDecreasesSizeWhenValid) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});

  auto res = table.Delete(1);

  EXPECT_TRUE(res.first);
  EXPECT_EQ(res.second.key, 1);
  EXPECT_EQ(res.second.value, 2);
  EXPECT_EQ(table.Size(), 0);
}

TEST_F(HashTableTest, FullIndicatesWhenTableIsFull) {
  TestTable table(2);

  EXPECT_TRUE(table.Insert(1, Value{1, 2}));
  EXPECT_TRUE(table.Insert(3, Value{3, 4}));

  EXPECT_TRUE(table.Full());
}

TEST_F(HashTableTest, InsertSucceedsEvenWhenTableIsFull) {
  TestTable table(2);

  table.Insert(1, Value{1, 2});
  table.Insert(3, Value{3, 4});
  
  EXPECT_TRUE(table.Insert(5, Value{5, 6}));
  EXPECT_EQ(table.Size(), 3);
}

TEST_F(HashTableTest, SearchDoesNotFindDeletedKey) {
  TestTable table(kCap);

  table.Insert(1, Value{1, 2});
  table.Delete(1);

  auto res = table.Search(1);

  EXPECT_FALSE(res.first);
}
}  // anonymous namespace
}  // namespace mp2
