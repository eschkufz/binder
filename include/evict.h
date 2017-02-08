#ifndef BINDER_INCLUDE_EVICT_H
#define BINDER_INCLUDE_EVICT_H

#include <set>

namespace binder {

// TODO: This isn't lru

template <typename S>
class Lru {
  public:
    void untouch(const typename S::k_type& k) {
      ks_.erase(k);
    }
    void touch(const typename S::k_type& k) {
      ks_.insert(k);
    }
    typename S::k_type evict() {
      return *ks_.begin();
    }
    friend void swap(Lru& lhs, Lru& rhs) {
      using std::swap;
      swap(lhs.ks_, rhs.ks_);
    }

  private:
    std::set<typename S::k_type> ks_;
};

} // namespace binder 

#endif
