#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include "include/evict.h"
#include "include/read.h"
#include "include/write.h"

namespace binder {

template <typename S1, typename S2,
          typename E = Lru<S1>, 
          typename R = Fetch<S2>, 
          typename W = WriteThrough<S2>>
class Cache {
  public:
    // TYPES:
    // Container:
    typedef typename S1::value_type value_type;
    typedef typename S1::reference reference;
    typedef typename S1::const_reference const_reference;
    typedef typename S1::iterator iterator;
    typedef typename S1::const_iterator const_iterator;
    typedef typename S1::difference_type difference_type;
    typedef typename S1::size_type size_type;
    // Other:
    typedef typename S1::k_type k_type;
    typedef typename S1::v_type v_type;
    
    // CONSTRUCT/COPY/DESTROY:
    // Container:
    Cache(S2* s2 = nullptr, size_t c = 16) : s1_(), s2_(s2), capacity_(c) { }
    Cache(const Cache& rhs) = default;
    Cache(Cache&& rhs) = default;
    Cache& operator=(const Cache& rhs) = default;
    Cache& operator=(Cache&& rhs) = default;
    ~Cache() = default;
    
    // ITERATORS:
    // Container:
    iterator begin() { 
      return s1_.begin();
    }
    const_iterator begin() const {
      return s1_.begin();
    }
    iterator end() {
      return s1_.end();
    }
    const_iterator end() const {
      return s1_.end();
    }
    const_iterator cbegin() const {
      return s1_.cbegin();
    }
    const_iterator cend() const {
      return s1_.cend();
    }

    // CAPACITY:
    // Container:
    bool empty() const {
      return s1_.empty();
    }
    size_type size() const {
      return s1_.size();
    }
    size_type max_size() const {
      return capacity_;
    }

    // MODIFIERS:
    // Container:
    void swap(Cache& rhs) {
      using std::swap;
      swap(s1_, rhs.s1_);
      swap(s2_, rhs.s2_);
      swap(capacity_, rhs.capacity_);
      swap(e_, rhs.e_);
      swap(r_, rhs.r_);
      swap(w_, rhs.w_);
    }

    // STORE INTERFACE:
    // Common:
    bool contains(const k_type& k) {
      return s1_.contains(k);
    }
    v_type get(const k_type& k) {
      if (s1_.contains(k)) {
        e_.touch(k);
      } else if (s2_ != nullptr) {
        r_.fetch(*s2_, k);
        for (auto v = r_.begin(), ve = r_.end(); v != ve; ++v) {
          put(*v);
        }
      }
      return s1_.get(k);
    }
    void put(const value_type& v) {
      s1_.put(v);
      e_.touch(v.first);
      w_.modify(*s2_, v);
      resize(max_size());
    }
    void erase(const k_type& k) {
      w_.flush(*s2_, k);
      e_.untouch(k);
      s1_.erase(k);
    }
    void clear() { 
      resize(0);
    }
    // Cache:
    void capacity(size_t c) {
      capacity_ = c;
      resize(max_size());
    }

    // COMPARISON:
    // Container:
    friend bool operator==(const Cache& lhs, const Cache& rhs) {
      return lhs.s1_ == rhs.s1_;
    }
    friend bool operator!=(const Cache& lhs, const Cache& rhs) {
      return !(lhs == rhs);
    }

    // SPECIALIZED ALGORITHMS:
    // Container:
    friend void swap(Cache& lhs, Cache& rhs) {
      lhs.swap(rhs);
    }

  private:
    S1 s1_;
    S2* s2_;
    size_t capacity_;
    E e_;
    R r_;
    W w_;

    void resize(size_t s) {
      while (size() > s) {
        erase(e_.evict());
      }
    }
};

} // namespace binder

#endif
