#include "gtest/gtest.h"
#include "include/redis.h"
#include "test/interface_test.h"

using namespace binder;

// Connection test
TEST(redis_store, connection) {
  // Default construction is disconnected
  RedisStore<char, int> s;
  EXPECT_FALSE(s.is_connected());

  // Connecting to a forbidden port should fail
  s.connect("localhost", 1);
  EXPECT_FALSE(s.is_connected());

  // Connecting twice successfully shouldn't be a problem
  s.connect("localhost", 6379);
  EXPECT_TRUE(s.is_connected());
  s.connect("localhost", 6379);
  EXPECT_TRUE(s.is_connected());

  // Disconnecting twice shouldn't be a problem
  s.disconnect();
  EXPECT_FALSE(s.is_connected());
  s.disconnect();
  EXPECT_FALSE(s.is_connected());

  // Copies should inherit connection
  s.connect("localhost", 6379);
  RedisStore<char, int> s1(s);
  EXPECT_TRUE(s1.is_connected());
  RedisStore<char, int> s2 = s;
  EXPECT_TRUE(s2.is_connected());

  // But closing the original connection shouldn't affect the copies
  s.disconnect();
  EXPECT_TRUE(s1.is_connected());
  EXPECT_TRUE(s2.is_connected());
}

// Basic test
TEST(redis_store, basic) {
  RedisStore<char, int> s("localhost", 6379);
  basic(s);
}

// Disconnected functionality test
TEST(redis_store, disconnected) {
  RedisStore<int, int> s("localhost", 6379);
  s.clear();
  s.put(make_pair(1,1));
  s.disconnect();

  // All iterators point to end
  EXPECT_EQ(s.begin(), s.end());

  // size() is zero
  EXPECT_TRUE(s.empty());
  EXPECT_EQ(s.size(), 0);

  // contains() returns false for everything
  EXPECT_FALSE(s.contains(1));
  // get() returns a default constructed value
  EXPECT_EQ(s.get(1), int());

  // put() doesn't do anything
  s.put(make_pair(2,2));
  s.connect("localhost", 6379);
  EXPECT_FALSE(s.contains(2));
  s.disconnect();
 
  // erase() doesn't do anything
  s.erase(1); 
  s.connect("localhost", 6379);
  EXPECT_TRUE(s.contains(1));
  s.disconnect();

  // clear() doesn't do anything
  s.clear();
  s.connect("localhost", 6379);
  EXPECT_TRUE(s.contains(1));
  s.disconnect();
}
