#include <iostream>
#include "include/binder.h"

using namespace binder;
using namespace std;

int main(int argc, char** argv) {
  if (argc != 3) {
    cout << "Usage: " << argv[0] << " <host> <port>" << endl;
    exit(1);  
  }

  return 0;
}
