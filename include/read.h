#ifndef BINDER_INCLUDE_READ_H
#define BINDER_INCLUDE_READ_H

namespace binder {

template <typename S1, typename S2>
struct Fetch {
  typename S2::iterator find(S1& s1, S2& s2, const typename S1::k_type& k) {
    return s2.find(k);
  }
  typename S2::iterator next(S1& s1, S2& s2) {
    return s2.end();
  }
  std::pair<typename S1::iterator, bool> insert(S1& s1, S2& s2, const typename S1::k_type& k, const typename S2::value_type val) {
    return s1.insert(val);
  }
};

} // namespace binder

#endif
