#ifndef BINDER_INCLUDE_EVICT_H
#define BINDER_INCLUDE_EVICT_H

namespace binder {

template <typename S1>
struct Lru {
  void touch(typename S1::iterator itr) {
    // Does nothing
  }
  typename S1::const_iterator evict(S1& s1) {
    return s1.cbegin();
  }
};

} // namespace binder 

#endif
