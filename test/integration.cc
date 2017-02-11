#include <algorithm>
#include "gtest/gtest.h"
#include "include/binder.h"
#include "test/interface.h"

using namespace binder;
using namespace std;

// README usage example
TEST(integration, README) {
  // A simple store
  Store<int, char> s1;
  s1.put(make_pair(1,'a'));
  s1.put(make_pair(2,'b'));
  s1.put(make_pair(3,'c'));

  // A simple unordered store
  UnorderedStore<int, char> s2;
  for (const auto& p : s1) {
    s2.put(p);
  }
  if (s2.contains(1)) {
    s2.erase(1);
  } else {
    s2.clear();
  }

  // A redis-backed store
  RedisStore<double, long> s3("localhost", 6379);
  s3.clear();

  // Change the exposed types of s2
  AdapterStore<double, long, decltype(s2)> s4(&s2);

  // Now that the types match, use s4 as a cache for s3
  Cache<decltype(s4), decltype(s3)> s5(&s4, &s3);

  // Do anything that you would do with an stl container!
  EXPECT_EQ(std::min_element(s5.begin(), s5.end())->second, s2.get(2));
  EXPECT_EQ(std::max_element(s5.begin(), s5.end())->second, s2.get(3));
}
