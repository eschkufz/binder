#include "gtest/gtest.h"
#include "include/adapter.h"
#include "include/store.h"
#include "test/interface_test.h"

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
