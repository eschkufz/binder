#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include "include/evict.h"
#include "include/read.h"
#include "include/write.h"

#include <utility>

namespace binder {

template <typename S1, typename S2,
          size_t C = 16,
          typename E = Lru<S1>, 
          typename R = Fetch<S1,S2>, 
          typename W = WriteThrough<S1,S2>>
class Cache {
  public:
    // TODO:
    // Iterators are going to have to invoke touch when they're derefed... how wild is that?

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
    // (default)
    
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
      return C;
    }

    // MODIFIERS:
    // Container:
    void swap(Cache& rhs) {
      swap(s1_, rhs.s1_);
      swap(s2_, rhs.s2_);
      // TODO: Need to swap the policies as well
    }

    // STORE INTERFACE:
    std::pair<iterator, bool> insert(const value_type& v) { 
      auto res = s1_.insert(v);
      dirty_insert(res.first);
      return res;
    }
    iterator erase(const_iterator position) { 
      w_.flush(s1_, s2_, *position);
      return s1_.erase(position); 
    }
    void clear() { 
      while (size() > 0) {
        erase(e_.evict(s1_));
      }
    }
    iterator find(const k_type& k) { 
      auto itr1 = s1_.find(k);
      if (itr1 != s1_.end()) {
        return itr1;
      }

      auto itr2 = r_.find(s1_, s2_, k);
      if (itr2 != s2_.end()) {
        itr1 = r_.insert(s1_, s2_, k, *itr2).first;
        clean_insert(itr1);
      }
      while ((itr2 = r_.next(s1_, s2_)) != s2_.end()) {
        auto tmp = r_.insert(s1_, s2_, k, *itr2).first;
        clean_insert(tmp);
      }
      return itr1;
    }

    // COMPARISON:
    // Container:
    friend bool operator==(const Cache& lhs, const Cache& rhs) {
      return lhs.s1_ == rhs.s1_;
    }
    friend bool operator!=(const Cache& lhs, const Cache& rhs) {
      return lhs.s1_ != rhs.s1_;
    }

    // SPECIALIZED ALGORITHMS:
    // Container:
    friend void swap(Cache& lhs, Cache& rhs) {
      lhs.swap(rhs);
    }

  private:
    S1 s1_;
    S2 s2_;
    E e_;
    R r_;
    W w_;

    void clean_insert(typename S1::iterator itr) {
      e_.touch(itr);
      w_.unset_dirty(s1_, s2_, itr);
      if (size() > max_size()) {
        erase(e_.evict(s1_));
      }
    }
    void dirty_insert(typename S1::iterator itr) {
      e_.touch(itr);
      w_.set_dirty(s1_, s2_, itr);
      if (size() > max_size()) {
        erase(e_.evict(s1_));
      }
    }
};

} // namespace binder

#endif
