#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include "include/database.h"

namespace binder {

template <typename T, typename W, typename F, typename E, typename S>
class Cache {
  public:
    typedef typename S::const_iterator const_iterator;

    Cache(Database& db, size_t c = 8) : capacity_(c), db_(db), fp_(db), wp_(db) { }

    size_t size() const {
      return s_.size();
    }
    size_t capacity() const {
      return capacity_;
    }

    const_iterator begin() const {
      return s_.begin();
    }
    const_iterator end() const {
      return s_.end();
    }

    void clear() {
      s_.clear();
    }
    bool contains(const typename T::key_type& k) {
      T::begin();

      const auto ck = T::kmap(k);
      const auto res = s_.find(ck) != s_.end();

      T::end();
      return res;
    }
    void fetch(const typename T::key_type& k) {
      T::begin();

      const auto ck = T::kmap(k);
      fetch_inner(ck);

      T::end();
    }
    void flush(const typename T::key_type& k) {
      T::begin();

      const auto ck = T::kmap(k);
      const auto itr = s_.find(ck);
      if (itr != s_.end()) {
        wp_.sync(itr);
      }

      T::end();
    }
    typename T::val_type get(const typename T::key_type& k) {
      T::begin();

      const auto ck = T::kmap(k);
      fetch_inner(ck);

      const auto cv = T::vinit(k, ck);
      const auto ins = s_.insert({ck, cv});
      if (ins.second) {
        wp_.write(ins.first);
        ep_.touch(ins.first);
        ensure_size();
      } 
      const auto res = T::vunmap(k, ck, ins.first->second);

      T::end();
      return res;
    }
    void put(const typename T::key_type& k, const typename T::val_type& v) {
      T::begin();

      const auto ck = T::kmap(k);
      auto itr = s_.find(ck);
      if (itr == s_.end()) {
        itr = s_.insert({ck, T::vinit()}).first;
      }
      const auto cv = T::vmap(v);
      T::merge(cv, itr->second);
      wp_.write(itr);
      ep_.touch(itr);
      ensure_size();

      T::end();
    }

  private:
    Database& db_;
    size_t capacity_;
    S s_;
    F fp_;
    W wp_;
    E ep_;

    void fetch_inner(const typename T::ckey_type& ck) {
      fp_.fetch(ck);
      for (const auto& res : fp_) {
        const auto ins = s_.insert(res);
        ep_.touch(ins.first);
        if (ins.second) {
          wp_.write(ins.first);
          ensure_size();
        } 
      }
    }
    void ensure_size() {
      if (size() > capacity()) {
        const auto itr = ep_.evict();
        wp_.sync(itr);
        s_.erase(itr);
      }
    }
};

} // namespace binder

#endif
