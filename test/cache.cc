#include "gtest/gtest.h"
#include "include/store.h"
#include "include/cache.h"
#include "test/interface_test.h"

using namespace binder;

// Basic policy tests
TEST(cache, basic) {
  typedef Store<char, int> S;
  Cache<S,S,26> s;

  basic_test(s);
}
TEST(cache, small) {
  typedef Store<int, int> S;
  Cache<S,S,1> s;

  s.insert(make_pair(0,0));
  for (size_t i = 0; i < 8; ++i) {
    s.insert(make_pair(i+1, i+1));
    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s.find(i), s.end());
    EXPECT_NE(s.find(i+1), s.end());
  }
}
