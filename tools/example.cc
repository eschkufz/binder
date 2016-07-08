#include <algorithm>
#include <cctype>
#include <iostream>
#include "include/binder.h"

using namespace binder;
using namespace std;

class MyCache : public Cache<string, int, string, int> {
  public:
    virtual ~MyCache() {}

  protected:
    virtual string cmap(const string& k) {
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
    virtual int init(const string& k, const string& ck) {
      (void) k;
      (void) ck;
      return 0;
    }
    virtual int vunmap(const string& k, const string& ck, const int& cv) {
      (void) k;
      (void) ck;
      return cv;
    }
};

int main(int argc, char** argv) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <host> <port>" << endl;
    exit(1);  
  }

  MyCache cache;
  cache.connect(argv[1], atoi(argv[2]));

  if (!cache.is_connected()) {
    cout << "Unable to connect to database at " << argv[1] << ":" << argv[2] << endl;
    exit(1);
  }
  cout << "Connected to database at " << argv[1] << ":" << argv[2] << endl;

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
