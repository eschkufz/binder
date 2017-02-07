#ifndef BINDER_INCLUDE_STORE_H
#define BINDER_INCLUDE_STORE_H

#include <map>
#include <unordered_map>

namespace binder {

template <typename K, typename V, typename C>
class AssocStore {
  public:
    // TYPES:
    // Container:
    typedef typename C::value_type value_type;
    typedef typename C::reference reference;
    typedef typename C::const_reference const_reference;
    typedef typename C::iterator iterator;
    typedef typename C::const_iterator const_iterator;
    typedef typename C::difference_type difference_type;
    typedef typename C::size_type size_type;
    // Other:
    typedef typename C::key_type k_type;
    typedef typename C::mapped_type v_type;
    
    // CONSTRUCT/COPY/DESTROY:
    // Container:
    // (default)
    
    // ITERATORS:
    // Container:
    iterator begin() {
      return c_.begin();
    }
    const_iterator begin() const {
      return c_.begin();
    }
    iterator end() {
      return c_.end();
    }
    const_iterator end() const {
      return c_.end();
    }
    const_iterator cbegin() const {
      return c_.cbegin();
    }
    const_iterator cend() const {
      return c_.cend();
    }

    // CAPACITY:
    // Container:
    bool empty() const {
      return c_.empty();
    }
    size_type size() const {
      return c_.size();
    }
    size_type max_size() const {
      return c_.max_size();
    }

    // MODIFIERS:
    // Container:
    void swap(AssocStore& rhs) {
      swap(c_, rhs.c_);
    }

    // STORE INTERFACE:
    // Common:
    bool contains(const k_type& k) {
      return c_.find(k) != c_.end();
    }
    v_type get(const k_type& k) {
      auto itr = c_.find(k);
      return itr == c_.end() ? v_type() : itr->second;
    }
    void put(const value_type& v) {
      c_.insert(v);
    }
    void erase(const k_type& k) {
      c_.erase(k);
    }
    void clear() {
      c_.clear();
    }

    // COMPARISON:
    // Container:
    friend bool operator==(const AssocStore& lhs, const AssocStore& rhs) {
      return lhs.c_ == rhs.c_;
    }
    friend bool operator!=(const AssocStore& lhs, const AssocStore& rhs) {
      return !(lhs == rhs);
    }

    // SPECIALIZED ALGORITHMS:
    // Container:
    friend void swap(AssocStore& lhs, AssocStore& rhs) {
      lhs.swap(rhs);
    }

  private:
    C c_;
};

template <typename K, typename V>
using Store = AssocStore<K,V,std::map<K,const V>>;
template <typename K, typename V>
using UnorderedStore = AssocStore<K,V,std::unordered_map<K,const V>>;

} // namespace binder

#endif
