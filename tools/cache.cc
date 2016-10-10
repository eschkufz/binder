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

  while (true) {
    cout << ">>> ";

    string s = "";
    cin >> s;

    if (s == "HELP") {
      cout << "  SIZE" << endl;
      cout << "  PRINT" << endl;
      cout << "  CLEAR" << endl;
      cout << "  FETCH_ALL" << endl;
      cout << "  FLUSH_ALL" << endl;
      cout << "  CONTAINS <string>" << endl;
      cout << "  FETCH <string>" << endl;
      cout << "  FLUSH <string>" << endl;
      cout << "  GET <string>" << endl;
      cout << "  PUT <string> <int>" << endl;
      cout << "  QUIT" << endl;
      continue;
    }
    if (s == "SIZE") {
      cout << "  " << cache.size() << endl;
      continue;
    }
    if (s == "PRINT") {
      size_t i = 0;
      for (const auto& line : cache) {
        cout << "  " << (++i) << ": \"" << line.key << "\" -> \"" << line.val << "\"" << endl;
      }
      continue;
    }
    if (s == "CLEAR") {
      cache.clear();
      cout << "  OK" << endl;
      continue;
    }
    if (s == "FETCH_ALL") {
      cache.fetch();
      cout << "  OK" << endl;
      continue;
    }
    if (s == "FLUSH_ALL") {
      cache.flush();
      cout << "  OK" << endl;
      continue;
    }
    if (s == "CONTAINS") {
      string k = "";
      cin >> k;
      cout << "  " << (cache.contains(k) ? "YES" : "NO") << endl;
      continue;
    }
    if (s == "FETCH") {
      string k = "";
      cin >> k;
      cache.fetch(k);
      cout << "  OK" << endl;
      continue;
    }
    if (s == "FLUSH") {
      string k = "";
      cin >> k;
      cache.flush(k);
      cout << "  OK" << endl;
      continue;
    }
    if (s == "GET") {
      string k = "";
      cin >> k;
      cout << "  \"" << k << "\" -> \"" << cache.get(k) << endl;
      continue;
    }
    if (s == "PUT") {
      string k = "";
      int v = 0;
      cin >> k >> v;
      cache.put(k, v);
      cout << "  OK" << endl;
      continue;
    }
    if (cin.eof() || s == "QUIT") {
      if (cin.eof()) {
        cout << endl;
      }
      cout << "  OK" << endl;
      break;
    }
    cout << "  UNRECOGNIZED COMMAND" << endl;
  }

  return 0;
}
