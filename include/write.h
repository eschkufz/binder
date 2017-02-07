#ifndef BINDER_INCLUDE_WRITE_H
#define BINDER_INCLUDE_WRITE_H

namespace binder {

template <typename S1, typename S2>
struct WriteThrough {
  void set_dirty(S1& s1, S2& s2, typename S1::iterator itr) {
    flush(s1, s2, *itr); 
  }
  void unset_dirty(S1& s1, S2& s2, typename S1::iterator itr) {
    // Does nothing.
  }
  void flush(S1& s1, S2& s2, const typename S1::value_type& v) {
    s2.insert(v); 
  }
};

} // namespace binder

#endif
