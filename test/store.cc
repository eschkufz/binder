#include "gtest/gtest.h"
#include "include/store.h"
#include "test/interface.h"

using namespace binder;

// Basic tests
TEST(store, basic) {
  Store<char, int> s;
  basic(s);
}
TEST(unordered_store, basic) {
  UnorderedStore<char, int> s;
  basic(s);
}
