#include <set>
#include <vector>
#include "gtest/gtest.h"
#include "include/database.h"

using namespace binder;
using namespace std;

// Basic functionality test
TEST(database, basic) {
  Database db;

  db.connect("localhost", 6379);
  EXPECT_TRUE(db.is_connected());

  db.clear();
  EXPECT_EQ(db.size(), 0);

  vector<Database::line_type> elems(db.begin(), db.end());
  EXPECT_EQ(elems.size(), 0);

  Database::str_type K = {"A", 1};
  Database::str_type v = {"1", 1};

  char str[1] = "";
  for (size_t i = 0; i < 26; ++i) {
    str[0] = (char)((int)'a' + i) ;
    Database::str_type k = {(const char*)&str, 1};

    db.put(k, v);
    EXPECT_EQ(db.size(), i+1);
    EXPECT_TRUE(db.contains(k));
    EXPECT_FALSE(db.contains(K));

    const auto res1 = db.get(k);
    EXPECT_EQ(res1.first.str[0], '1');
    EXPECT_EQ(res1.first.len, 1);
    EXPECT_TRUE(res1.second);

    const auto res2 = db.get(K);
    EXPECT_FALSE(res2.second);
    
    set<char> keys;
    for (const auto& line : db) {
      EXPECT_EQ(line.first.len, 1);
      keys.insert(line.first.str[0]);
    }
    for (size_t j = 0; j < i; ++j) {
      const auto c = (char)((int)'a' + j);
      EXPECT_TRUE(keys.find(c) != keys.end());
    }
  }
}

