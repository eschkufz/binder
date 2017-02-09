#include "gtest/gtest.h"
#include "include/cache.h"
#include "include/store.h"
#include "test/interface_test.h"

using namespace binder;

// Basic policy tests
TEST(cache, basic) {
  Store<char, int> ci;
  Cache<decltype(ci),decltype(ci)> s(&ci, 26);
  basic(s);
}

// Evict policy tests
TEST(cache, lru) {
  Store<int, int> ii;
  Cache<decltype(ii),decltype(ii)> s(&ii, 1);

  s.put(make_pair(0,0));
  for (size_t i = 0; i < 8; ++i) {
    s.put(make_pair(i+1, i+1));
    EXPECT_EQ(s.size(), 1);
    EXPECT_FALSE(s.contains(i));
    EXPECT_TRUE(s.contains(i+1));
  }
}

// Read policy tests
TEST(cache, fetch) {
  Store<int, int> ii;
  Cache<
    Store<int,int>,
    Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteThrough<Store<int,int>>> s(&ii);

  ii.put(make_pair(1,1));
  EXPECT_FALSE(s.contains(1));
  s.get(1);
  EXPECT_TRUE(s.contains(1));
  s.get(2);
  EXPECT_FALSE(s.contains(2));
}

// Write policy tests
TEST(cache, write_through) {
  Store<int, int> ii;
  Cache<
    Store<int,int>,
    Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteThrough<Store<int,int>>> s(&ii);

  s.put(make_pair(1,1));
  EXPECT_TRUE(ii.contains(1));
  EXPECT_TRUE(s.contains(1));
}
TEST(cache, write_back) {
  Store<int, int> ii;
  Cache<
    Store<int,int>,
    Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteBack<Store<int,int>>> s(&ii);

  s.put(make_pair(1,1));
  EXPECT_FALSE(ii.contains(1));
  EXPECT_TRUE(s.contains(1));

  s.clear();

  EXPECT_TRUE(ii.contains(1));
  EXPECT_FALSE(s.contains(1));
}
