#ifndef BINDER_INCLUDE_WRITE_H
#define BINDER_INCLUDE_WRITE_H

#include <map>

namespace binder {

template <typename S>
struct WriteThrough {
  void modify(S& s, const typename S::value_type& v) {
    s.put(v);
  }
  void flush(S& s, const typename S::k_type& k) {
    // Does nothing.
  }
  friend void swap(WriteThrough& lhs, WriteThrough& rhs) {
    // Does nothing.
  }
};

template <typename S>
class WriteBack {
  public:
    void modify(S& s, const typename S::value_type& v) {
      vs_.insert(v);
    }
    void flush(S& s, const typename S::k_type& k) {
      auto itr = vs_.find(k);
      if (itr != vs_.end()) {
        s.put(*itr);
        vs_.erase(itr);
      }
    }
    friend void swap(WriteBack& lhs, WriteBack& rhs) {
      using std::swap;
      swap(lhs.vs_, rhs.vs_);
    }

  private:
    std::map<typename S::k_type, typename S::value_type> vs_;
};

} // namespace binder

#endif
