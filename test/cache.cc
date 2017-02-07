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

// Adapter tests
TEST(adapter, basic) {
  typedef Store<char, int> S1;
  typedef Store<int, double> S2;
  typedef Cast<char, int, int, double> M;
  Cache<S1,S2,26,Lru<S1>,MFetch<S1,S2,M>,MWriteThrough<S1,S2,M>> s;

  basic_test(s);
}
TEST(adapter, one_to_one) {
  typedef Store<int, int> S;
  struct M {
    int kmap(int dk) { return dk; }
    int vmap(int dv) { return dv + 1; }
    int vunmap(int dk, int rk, int rv) { return rv - 1; }
  };
  Cache<S,S,16,Lru<S>,MFetch<S,S,M>,MWriteThrough<S,S,M>> s;

  auto itr = s.insert(make_pair(1,1)).first;
  EXPECT_EQ(itr->first, 1);
  EXPECT_EQ(itr->second, 1);
}
TEST(adapter, many_to_one) {
  typedef Store<int, int> S;
  struct M {
    int kmap(int dk) { return dk % 2; }
    int vmap(int dv) { return dv + 1; }
    int vunmap(int dk, int rk, int rv) { return rv - 1; }
  };
  Cache<S,S,16,Lru<S>,MFetch<S,S,M>,MWriteThrough<S,S,M>> s;

  auto itr = s.insert(make_pair(1,1)).first;
  EXPECT_EQ(s.size(), 1);
  EXPECT_EQ(itr->first, 1);
  EXPECT_EQ(itr->second, 1);

  itr = s.find(3);
  EXPECT_EQ(s.size(), 2);
  EXPECT_EQ(itr->first, 3);
  EXPECT_EQ(itr->second, 1);
}
