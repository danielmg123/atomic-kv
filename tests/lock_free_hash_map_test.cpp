#include "../include/lock_free_hash_map.hpp"

#include <gtest/gtest.h>

using kv::LockFreeHashMap;

TEST(LockFreeHashMapTest, GetOnEmpty) {
  LockFreeHashMap<int, int, 16> map;
  EXPECT_EQ(map.get(42), std::nullopt);
}

TEST(LockFreeHashMapTest, SinglePutGet) {
  LockFreeHashMap<int, std::string, 16> map;
  EXPECT_TRUE(map.put(1, "one"));
  auto val = map.get(1);
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(val.value(), "one");
}

TEST(LockFreeHashMapTest, ReplaceValue) {
  LockFreeHashMap<int, int, 8> map;
  EXPECT_TRUE(map.put(2, 20));
  EXPECT_TRUE(map.put(2, 30));
  auto val = map.get(2);
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(val.value(), 30);
}

TEST(LockFreeHashMapTest, EraseExisting) {
  LockFreeHashMap<int, int, 8> map;
  map.put(3, 300);
  EXPECT_TRUE(map.erase(3));
  EXPECT_EQ(map.get(3), std::nullopt);
}

TEST(LockFreeHashMapTest, EraseNonExisting) {
  LockFreeHashMap<int, int, 8> map;
  EXPECT_FALSE(map.erase(99));
}

TEST(LockFreeHashMapTest, MultipleKeys) {
  LockFreeHashMap<int, int, 4> map;
  EXPECT_TRUE(map.put(1, 100));
  EXPECT_TRUE(map.put(2, 200));
  EXPECT_TRUE(map.put(3, 300));
  EXPECT_EQ(map.get(1).value_or(-1), 100);
  EXPECT_EQ(map.get(2).value_or(-1), 200);
  EXPECT_EQ(map.get(3).value_or(-1), 300);
}

auto main(int argc, char** argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}