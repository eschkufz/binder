#include <iostream>
#include "include/binder.h"

using namespace binder;
using namespace std;

int main() {
  Database db;
  db.connect("localhost", 6379);
  if (!db.is_connected()) {
    cerr << "Unable to connect to database at localhost:6379" << endl;
    exit(1);
  }

  while (true) {
    cout << ">>> ";

    string s = "";
    cin >> s;

    if (s == "HELP") {
      cout << "  SIZE" << endl;
      cout << "  PRINT" << endl;
      cout << "  CLEAR" << endl;
      cout << "  CONTAINS <key>" << endl;
      cout << "  GET <key>" << endl;
      cout << "  PUT <key> <val>" << endl;
      cout << "  QUIT" << endl;
      continue;
    }
    if (s == "SIZE") {
      cout << "  " << db.size() << endl;
      continue;
    }
    if (s == "PRINT") {
      size_t i = 0;
      for (const auto& line : db) {
        const string key(line.first.str, line.first.len);
        const string val(line.second.str, line.second.len);

        cout << "  " << (++i) << ": \"" << key << "\" -> \"" << val << "\"" << endl;
      }
      continue;
    }
    if (s == "CLEAR") {
      db.clear();
      cout << "  OK" << endl;
      continue;
    }
    if (s == "CONTAINS") {
      string k = "";
      cin >> k;
      cout << "  " << (db.contains({k.c_str(), k.length()}) ? "YES" : "NO") << endl;
      continue;
    }
    if (s == "GET") {
      string k = "";
      cin >> k;
      const auto res = db.get({k.c_str(), k.length()});
      cout << "  \"" << (res.second ? string(res.first.str, res.first.len) : "(nil)") << "\"" << endl;
      continue;
    }
    if (s == "PUT") {
      string k = "";
      string v = "";
      cin >> k >> v;
      db.put({k.c_str(), k.length()}, {v.c_str(), v.length()});
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
