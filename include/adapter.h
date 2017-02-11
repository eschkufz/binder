#ifndef BINDER_INCLUDE_ADAPTER_H
#define BINDER_INCLUDE_ADAPTER_H

namespace binder {

template <typename DK, typename DV, typename RK, typename RV>
struct Cast {
  RK kmap(const DK& dk) {
    return (RK) dk;
  }
  RV vmap(const DV& dv) {
    return (RV) dv;
  }
  DK kunmap(const RK& rk) {
    return (DK) rk;
  }
  DV vunmap(const RV& rv) {
    return (DV) rv;
  }
  DV vunmap(const DK& dk, const RK& rk, const RV& rv) {
    (void) dk;
    (void) rk;
    return (DV) rv;
  } 
};

template <typename K, typename V, typename S, 
          typename M=Cast<K,V,typename S::k_type,typename S::v_type>>
class AdapterStore {
  public:
    template <bool is_const>
    class Iterator {
      friend class AdapterStore;

      // TYPES:
      private:
        typedef typename AdapterStore::reference ref;
        typedef typename AdapterStore::value_type* ptr;
      public:
        typedef typename AdapterStore::value_type value_type;
        typedef typename std::conditional<is_const, const ref, ref>::type reference;
        typedef typename std::conditional<is_const, const ptr, ptr>::type pointer;
        typedef typename AdapterStore::difference_type difference_type;
        typedef typename std::forward_iterator_tag iterator_category;

      // CONSTRUCT/COPY/DESTROY:
      private:
        Iterator(typename S::const_iterator itr) : itr_(itr), val_(nullptr) { }
      public:
        Iterator() : val_(nullptr) { }
        Iterator(const Iterator& rhs) : itr_(rhs.itr_), val_(nullptr) { }
        Iterator(Iterator&& rhs) : Iterator() {
          swap(rhs);
        }
        Iterator& operator=(Iterator rhs) {
          swap(rhs);
          return *this;
        }
        ~Iterator() { 
          if (val_ != nullptr) {
            delete val_;
          }
        }

        // ABILITIES:
        reference operator*() {
          get();
          return *val_;
        }
        pointer operator->() {
          get();
          return val_;
        }
        Iterator& operator++() {
          ++itr_;
          return *this;
        }
        Iterator operator++(int) {
          auto ret = *this;
          ++(*this);
          return ret;
        }
        bool operator==(const Iterator& rhs) const {
          return itr_ == rhs.itr_;
        }
        bool operator!=(const Iterator& rhs) const {
          return !(*this == rhs);
        }

      private:
        typename S::const_iterator itr_;
        value_type* val_;

        void get() {
          if (val_ != nullptr) {
            delete val_;
          }
          M m;
          val_ = new value_type(m.kunmap(itr_->first), m.vunmap(itr_->second));
        }
        void swap(Iterator& rhs) {
          using std::swap;
          swap(itr_, rhs.itr_);
          swap(val_, rhs.val_);
        }
    };

    // TYPES:
    // Container:
    typedef std::pair<const K, const V> value_type;
    typedef value_type& reference;
    typedef const reference const_reference;
    typedef Iterator<false>  iterator;
    typedef Iterator<true> const_iterator;
    typedef typename S::difference_type difference_type;
    typedef typename S::size_type size_type;
    // Other:
    typedef K k_type;
    typedef V v_type;
    
    // CONSTRUCT/COPY/DESTROY:
    // Container:
    AdapterStore(S* s = nullptr) : s_(s) { }
    AdapterStore(const AdapterStore& rhs) = default;
    AdapterStore(AdapterStore&& rhs) = default;
    AdapterStore& operator=(const AdapterStore& rhs) = default;
    AdapterStore& operator=(AdapterStore&& rhs) = default;
    ~AdapterStore() = default;

    // ITERATORS:
    // Container:
    iterator begin() {
      return s_ != nullptr ? iterator(s_->begin()) : iterator();
    }
    const_iterator begin() const {
      return s_ != nullptr ? const_iterator(s_->begin()) : const_iterator();
    }
    iterator end() {
      return s_ != nullptr ? iterator(s_->end()) : iterator();
    }
    const_iterator end() const {
      return s_ != nullptr ? const_iterator(s_->end()) : const_iterator();
    }
    const_iterator cbegin() const {
      return s_ != nullptr ? const_iterator(s_->begin()) : const_iterator();
    }
    const_iterator cend() const {
      return s_ != nullptr ? const_iterator(s_->end()) : const_iterator();
    }

    // CAPACITY:
    // Container:
    bool empty() const {
      return s_ != nullptr ? s_->empty() : true;
    }
    size_type size() const {
      return s_ != nullptr ? s_->size() : 0;
    }
    size_type max_size() const {
      return s_ != nullptr ? s_->max_size() : 0;
    }

    // MODIFIERS:
    // Container:
    void swap(AdapterStore& rhs) {
      using std::swap;
      swap(s_, rhs.s_);
    }

    // STORE INTERFACE:
    // Common:
    bool contains(const k_type& dk) {
      return s_ != nullptr ? s_->contains(M().kmap(dk)) : false;
    }
    v_type get(const k_type& dk) {
      if (s_ == nullptr) {
        return v_type();
      }
      M m;
      const auto rk = m.kmap(dk);
      const auto rv = s_->get(rk);
      return m.vunmap(dk, rk, rv);
    }
    void put(const value_type& v) {
      if (s_ != nullptr) {
        M m;
        s_->put(std::make_pair(m.kmap(v.first), m.vmap(v.second)));
      }
    }
    void erase(const k_type& dk) {
      if (s_ != nullptr) {
        s_->erase(M().kmap(dk)); 
      }
    }
    void clear() {
      if (s_ != nullptr) {
        s_->clear();
      }
    }
    // AdapterStore:
    S* backing_store(S* s = nullptr) {
      auto ret = s_;
      if (s != nullptr) {
        s_ = s;
      }
      return ret;
    }

    // COMPARISON:
    // Container:
    friend bool operator==(const AdapterStore& lhs, const AdapterStore& rhs) {
      return lhs->s_ == rhs->s_;
    }
    friend bool operator!=(const AdapterStore& lhs, const AdapterStore& rhs) {
      return !(lhs == rhs);
    }

    // SPECIALIZED ALGORITHMS:
    // Container:
    friend void swap(AdapterStore& lhs, AdapterStore& rhs) {
      lhs.swap(rhs);
    }

  private:
    S* s_;
};

} // namespace binder

#endif
