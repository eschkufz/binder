#include "gtest/gtest.h"
#include "include/adapter.h"
#include "include/store.h"
#include "test/interface.h"

using namespace binder;

// Basic test
TEST(adapter_store, basic) {
  Store<int, double> id;
  AdapterStore<char, int, decltype(id)> s(&id);
  basic(s);
}

// Backing store test
TEST(adapter_store, backing_store) {
  Store<int, int> ii;
  AdapterStore<int, int, decltype(ii)> s;
  ii.put(make_pair(1,1));

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

  s.backing_store(&ii);
  EXPECT_FALSE(s.contains(2));
  EXPECT_TRUE(s.contains(1));
}

// Mapping tests
TEST(adapter_store, one_to_one) {
  struct M {
    int kmap(int dk) { return dk + 1; }
    int vmap(int dv) { return dv + 1; }
    int kunmap(int rk) { return rk - 1; }
    int vunmap(int rv) { return rv - 1; }
    int vunmap(int dk, int rk, int rv) { return rv - 1; }
  };
  Store<int, int> ii;
  AdapterStore<int,int,decltype(ii),M> s(&ii);

  s.put(make_pair(1,1));
  EXPECT_TRUE(s.contains(1));
  EXPECT_EQ(s.get(1), 1);
}
TEST(adapter, many_to_one) {
  struct M {
    int kmap(int dk) { return dk % 2; }
    int vmap(int dv) { return dv + 1; }
    int kunmap(int rk) { return rk; }
    int vunmap(int rv) { return rv - 1; }
    int vunmap(int dk, int rk, int rv) { return rv - 1; }
  };
  Store<int, int> ii;
  AdapterStore<int,int,decltype(ii),M> s(&ii);

  s.put(make_pair(1,1));
  s.put(make_pair(3,1));
  EXPECT_EQ(s.size(), 1);
  EXPECT_EQ(s.get(1), 1);
  EXPECT_EQ(s.get(3), 1);
}
