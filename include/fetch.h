#ifndef BINDER_INCLUDE_FETCH_H
#define BINDER_INCLUDE_FETCH_H

#include <sstream>
#include <vector>
#include "include/database.h"

namespace binder {

template <typename T, typename S>
class Fetch {
  public:
    typedef typename std::vector<std::pair<typename T::ckey_type, typename T::cval_type>>::const_iterator const_iterator;

    Fetch(Database& db) : db_(db) { }

    void fetch(const typename T::ckey_type& ck) {
      res_.clear();

      std::stringstream ks;
      T::kwrite(ks, ck);
      const auto res = db_.get({ks.str().c_str(), ks.str().length()});
      if (res.second) {
        auto cv = T::vinit();
        std::stringstream vs({res.first.str, res.first.len}); 
        T::vread(vs, cv);
        res_.push_back({ck, cv});
      }
    }
    const_iterator begin() const {
      return res_.begin();
    }
    const_iterator end() const {
      return res_.end();
    }

  private:
    Database& db_;
    std::vector<std::pair<typename T::ckey_type, typename T::cval_type>> res_;
};

} // namespace binder

#endif
