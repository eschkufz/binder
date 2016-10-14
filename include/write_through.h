#ifndef BINDER_INCLUDE_WRITE_THROUGH_H
#define BINDER_INCLUDE_WRITE_THROUGH_H

#include <sstream>
#include "include/database.h"

namespace binder {

template <typename T, typename S>
class WriteThrough {
  public:
    WriteThrough(Database& db) : db_(db) { }

    void write(typename S::const_iterator line) {
      std::stringstream ks;
      std::stringstream vs;
      T::kwrite(ks, line->first);
      T::vwrite(vs, line->second);
      db_.put({ks.str().c_str(), ks.str().length()}, {vs.str().c_str(), vs.str().length()});
    }
    void sync(typename S::const_iterator line) {
      (void) line;
    }

  private:
    Database& db_;
};

} // namespace binder

#endif
