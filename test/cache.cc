#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "include/cache.h"
#include "include/database.h"

using namespace binder;
using namespace std;

class MyCache : public Cache<string, int, string, int> {
  public:
    MyCache(Database* db) : Cache(db) { }
    ~MyCache() override {}

  protected:
    string kmap(const string& k) override {
      return k;
    }
    int vmap(const int& v) override {
      return v;
    }
    void merge(const int& v1, int& v2) override {
      v2 += v1;
    }
};

// Basic functionality test
TEST(cache, basic) {
  Database db;
  MyCache cache(&db);
  db.connect("localhost", 6379);
  EXPECT_TRUE(db.is_connected());

  cache.clear();
  EXPECT_EQ(cache.size(), 0);

  vector<MyCache::line_type> elems(cache.begin(), cache.end());
  EXPECT_EQ(elems.size(), 0);

  // TODO: Add need some non-trivial tests here
}
