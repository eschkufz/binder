#include "gtest/gtest.h"
#include "include/cache.h"
#include "include/store.h"
#include "test/interface.h"

using namespace binder;

// Missing stores test
TEST(cache, missing_stores) {
  Store<int,int> p;
  Store<int,int> b;
  Cache<Store<int,int>, Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteThrough<Store<int,int>>> s;

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
  // erase() doesn't do anything
  s.erase(1); 
  // clear() doesn't do anything
  s.clear();
}

// Basic policy tests
TEST(cache, basic) {
  Store<char, int> ci1;
  Store<char, int> ci2;
  Cache<decltype(ci1),decltype(ci2)> s(&ci1, &ci2, 26);
  basic(s);
}

// Evict policy tests
TEST(cache, lru) {
  Store<int, int> ii1;
  Store<int, int> ii2;
  Cache<decltype(ii1),decltype(ii2)> s(&ii1, &ii2, 1);

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
  Store<int, int> ii1;
  Store<int, int> ii2;
  Cache<
    Store<int,int>,
    Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteThrough<Store<int,int>>> s(&ii1, &ii2);

  ii2.put(make_pair(1,1));
  EXPECT_FALSE(s.contains(1));
  s.get(1);
  EXPECT_TRUE(s.contains(1));
  s.get(2);
  EXPECT_FALSE(s.contains(2));
}

// Write policy tests
TEST(cache, write_through) {
  Store<int, int> ii1;
  Store<int, int> ii2;
  Cache<
    Store<int,int>,
    Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteThrough<Store<int,int>>> s(&ii1, &ii2);

  s.put(make_pair(1,1));
  EXPECT_TRUE(ii2.contains(1));
  EXPECT_TRUE(s.contains(1));
}
TEST(cache, write_back) {
  Store<int, int> ii1;
  Store<int, int> ii2;
  Cache<
    Store<int,int>,
    Store<int,int>,
    Lru<Store<int,int>>,
    Fetch<Store<int,int>>,
    WriteBack<Store<int,int>>> s(&ii1, &ii2);

  s.put(make_pair(1,1));
  EXPECT_FALSE(ii2.contains(1));
  EXPECT_TRUE(s.contains(1));

  s.clear();

  EXPECT_TRUE(ii2.contains(1));
  EXPECT_FALSE(s.contains(1));
}
