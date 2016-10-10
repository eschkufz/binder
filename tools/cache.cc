#include <algorithm>
#include <cctype>
#include <iostream>
#include "include/binder.h"

using namespace binder;
using namespace std;

class MyCache : public Cache<string, int, string, int> {
  public:
    MyCache(Database* db) : Cache(db) { }
    virtual ~MyCache() {}

  protected:
    virtual string kmap(const string& k) {
      auto ret = k;
      transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
      return ret;
    }
    virtual int vmap(const int& v) {
      return v;
    }
    virtual void merge(const int& v1, int& v2) {
      v2 += v1;
    }
};

int main() {
  Database db;
  db.connect("localhost", 6379);
  if (!db.is_connected()) {
    cerr << "Unable to connect to database at localhost:6379" << endl;
    exit(1);
  }
  MyCache cache(&db);
  cache.write_through();

  string s = "";
  while (cin >> s) {
    cout << (cache.contains(s) ? "[   ] " : "[new] ");
    cout << "\"" << s << "\" ";
    cout << cache.get(s);
    cache.put(s, 1);
    cout << " -> ";
    cout << cache.get(s);
    cout << endl;
  }

  return 0;
}
