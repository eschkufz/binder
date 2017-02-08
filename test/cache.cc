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

// Small cache
TEST(cache, small) {
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
