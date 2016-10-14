#ifndef BINDER_INCLUDE_WRITE_BACK_H
#define BINDER_INCLUDE_WRITE_BACK_H

#include <set>
#include "include/write_through.h"

namespace binder {

template <typename T, typename S>
class WriteBack {
  public:
    WriteBack(Database& db) : db_(db) { }

    void write(typename S::const_iterator line) {
      dirty_.insert(line->first);
    }
    void sync(typename S::const_iterator line) {
      const auto itr = dirty_.find(line->first);
      if (itr == dirty_.end()) {
        return;
      }
      WriteThrough<T,S>(db_).write(line);
      dirty_.erase(itr);
    }

  private:
    Database& db_;
    std::set<typename T::ckey_type> dirty_;
};

} // namespace binder

#endif
