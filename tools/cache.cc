#include <algorithm>
#include <cctype>
#include <iostream>
#include "include/binder.h"

using namespace binder;
using namespace std;

struct MyTypes {
  typedef string key_type;
  typedef int val_type;
  typedef string ckey_type;
  typedef int cval_type;

  static ckey_type kinit() {
    return "";
  }
  static cval_type vinit() {
    return 0;
  }
  static cval_type vinit(const key_type& k, const ckey_type& ck) {
    return 0;
  }

  static ckey_type kmap(const key_type& k) {
    auto ret = k;
    transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
    return ret;
  }
  static cval_type vmap(const val_type& v) {
    return v;
  }
  static void merge(const cval_type& v1, cval_type& v2) {
    v2 += v1;
  }
  static val_type vunmap(const key_type& k, const ckey_type& ck, const cval_type& cv) {
    (void) k;
    (void) ck;
    return cv;
  }

  static void kwrite(std::ostream& os, const ckey_type& k) {
    os << k;
  }
  static void vwrite(std::ostream& os, const cval_type& v) {
    os << v;
  }
  static void kread(std::istream& is, ckey_type& k) {
    is >> k;
  }
  static void vread(std::istream& is, cval_type& v) {
    is >> v;
  }

  static void begin() { }
  static void end() { }
};

int main() {
  Database db;
  db.connect("localhost", 6379);
  if (!db.is_connected()) {
    cerr << "Unable to connect to database at localhost:6379" << endl;
    exit(1);
  }
  Cache<MyTypes,
    WriteBack<MyTypes, Storage<MyTypes>>,
    Fetch<MyTypes, Storage<MyTypes>>,
    Clock<MyTypes, Storage<MyTypes>>,
    Storage<MyTypes>      
  > cache(db, 4);

  while (true) {
    cout << ">>> ";

    string s = "";
    cin >> s;

    if (s == "HELP") {
      cout << "  SIZE" << endl;
      cout << "  PRINT" << endl;
      cout << "  CLEAR" << endl;
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
        cout << "  " << (++i) << ": \"" << line.first << "\" -> \"" << line.second << "\"" << endl;
      }
      continue;
    }
    if (s == "CLEAR") {
      cache.clear();
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
