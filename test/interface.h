#ifndef BINDER_TEST_INTERFACE_TEST_H
#define BINDER_TEST_INTERFACE_TEST_H

#include <set>
using namespace std;

template <typename S>
void basic(S& s) {
  s.clear();
  EXPECT_EQ(s.size(), 0);

  typename S::k_type K = 'A';
  typename S::v_type v = 1;

  for (size_t i = 0; i < 26; ++i) {
    char k = (char)((int)'a' + i);
    
    s.put(make_pair(k,v));
    EXPECT_EQ(s.size(), i+1);
    EXPECT_TRUE(s.contains(k));
    EXPECT_FALSE(s.contains(K));
    EXPECT_EQ(s.get(k), v);

    for (auto i = s.cbegin(), ie = s.cend(); i != ie; ++i) {
      EXPECT_EQ(i->second, v);
    }

    set<char> keys;
    for (const auto& val : s) {
      keys.insert(val.first);
    }
    for (size_t j = 0; j < i; ++j) {
      const auto c = (char)((int)'a' + j);
      EXPECT_NE(keys.find(c), keys.end());
    }
  }
}

#endif
