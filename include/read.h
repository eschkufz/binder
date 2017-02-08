#ifndef BINDER_INCLUDE_READ_H
#define BINDER_INCLUDE_READ_H

#include <vector>

namespace binder {

template <typename S>
class Fetch {
  public:
    typedef typename std::vector<typename S::value_type>::const_iterator const_iterator;

    void fetch(S& s, const typename S::k_type& k) {
      vs_.clear();
      if (s.contains(k)) {
        vs_.push_back(std::make_pair(k,s.get(k)));
      }
    }
    const_iterator begin() {
      return vs_.begin();
    }
    const_iterator end() {
      return vs_.end();
    }
    friend void swap(Fetch& lhs, Fetch& rhs) {
      using std::swap;
      swap(lhs.vs_, rhs.vs_);
    }

  private:
    std::vector<typename S::value_type> vs_;
};

} // namespace binder

#endif
