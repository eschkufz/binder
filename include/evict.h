#ifndef BINDER_INCLUDE_EVICT_H
#define BINDER_INCLUDE_EVICT_H

#include <list>
#include <map>

namespace binder {

template <typename S>
class Lru {
  public:
    void erase(const typename S::k_type& k) {
      auto itr = index_.find(k);
      lru_.erase(itr->second);
      index_.erase(itr);
    }
    void touch(const typename S::k_type& k) {
      lru_.push_front(k);

      auto itr = index_.find(k);
      if (itr != index_.end()) {
        lru_.erase(itr->second);
        itr->second = lru_.begin();
      } else {
        index_.insert(itr, std::make_pair(k, lru_.begin()));
      }
    }
    typename S::k_type evict() {
      return lru_.back();
    }
    friend void swap(Lru& lhs, Lru& rhs) {
      using std::swap;
      swap(lhs.lru_, rhs.lru_);
      swap(lhs.index_, rhs.index_);
    }

  private:
    std::list<typename S::k_type> lru_;
    std::map<typename S::k_type, typename std::list<typename S::k_type>::iterator> index_;
};

} // namespace binder 

#endif
