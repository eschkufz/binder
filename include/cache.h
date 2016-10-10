#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include <cassert>
#include <sstream>
#include <unordered_map>
#include "include/database.h"

namespace binder {

template <typename Key, typename Val, typename CKey, typename CVal>
class Cache {
  private:
    struct meta_type {
      CVal cval;
      bool is_dirty;
    };

  public:
    struct line_type {
      CKey key;
      CVal val;
    };

    class const_iterator : public std::iterator<std::forward_iterator_tag, line_type> {
      public:
        const_iterator() : itr_() { }
        const_iterator(typename std::unordered_map<CKey, meta_type>::const_iterator itr) : itr_(itr) { }
        const_iterator(const_iterator& rhs) : itr_(rhs.itr_) { }
        const_iterator(const_iterator&& rhs) : const_iterator() {
          swap(*this, rhs);
        }
        const_iterator& operator=(const_iterator rhs) {
          swap(*this, rhs);
          return *this;
        }

        const_iterator& operator++() {
          itr_++;
          return *this;
        }
        const_iterator operator++(int) {
          const auto ret = *this;
          ++(*this);
          return ret;
        }

        bool operator==(const const_iterator& rhs) const {
          return itr_ == rhs.itr_;
        }
        bool operator!=(const const_iterator& rhs) const {
          return !(*this == rhs);
        }

        line_type operator*() const {
          return {itr_->first, itr_->second.cval};
        }
        line_type* operator->() const {
          assert(false);
          return nullptr;
        }
      
        friend void swap(const_iterator& lhs, const_iterator& rhs) {
          using std::swap;
          swap(lhs.itr_, rhs.itr_);
        }
      private: 
        typename std::unordered_map<CKey, meta_type>::const_iterator itr_;
    };
    

    Cache(Database* db) : db_(db), wt_(false) { }
    Cache(const Cache& rhs) : db_(rhs.db_), contents_(rhs.contents_), wt_(rhs.wt_) { }
    Cache(Cache&& rhs) : Cache() {
      swap(*this, rhs);
    }    
    Cache& operator=(Cache rhs) {
      swap(*this, rhs);
      return *this;
    }
    virtual ~Cache() { }

    Cache& write_through(bool wt = true) {
      wt_ = wt;
      return *this;
    }

    Database& get_database() const {
      return *db_;
    }

    size_t size() const {
      return contents_.size();
    }
    const_iterator begin() const {
      return const_iterator(contents_.begin());
    }
    const_iterator end() const {
      return const_iterator(contents_.end());
    }

    void clear() {
      contents_.clear();
    }
    void fetch() {
      auto ck = kinit();
      for (const auto& line : *db_) {
        std::stringstream ks({line.key.str, line.key.len});
        kread(ks, ck);

        const auto itr = contents_.find(ck);
        if (itr == contents_.end() || !itr->second.is_dirty) {
          auto cv = vinit();
          std::stringstream vs({line.val.str, line.val.len});
          vread(vs, cv);

          contents_.insert({ck, {cv,false}});
          evict();
        }
      }
    }
    void flush() {
      for (auto& line : contents_) {
        if (line.second.is_dirty) {
          db_set(line.first, line.second.cval);
          line.second.is_dirty = false;
        }
      }
    }

    virtual bool contains(const Key& k) {
      const auto ck = kmap(k);
      return contents_.find(ck) != contents_.end();
    }
    virtual void fetch(const Key& k) {
      const auto ck = kmap(k);
      auto cv = vinit(k, ck);
      if (db_get(ck, cv)) {
        contents_[ck] = {cv,false};
        evict();
      }
    }
    virtual void flush(const Key& k) {
      const auto ck = kmap(k);
      const auto itr = contents_.find(ck);
      if (itr != contents_.end() && itr->second.is_dirty) {
        db_set(ck, itr->second.cval);
        itr->second.is_dirty = false;
      }
    }
    virtual Val get(const Key& k) {
      const auto ck = kmap(k);
      auto itr = contents_.find(ck);
      if (itr == contents_.end()) {
        auto cv = vinit(k, ck);
        const auto res = db_get(ck, cv);
        itr = contents_.insert({ck, {cv,!res}}).first;
        evict();
      }

      const auto v = vunmap(k, ck, itr->second.cval);
      return v;
    }
    virtual void put(const Key& k, const Val& v) {
      const auto ck = kmap(k);
      const auto cv = vmap(v);
      auto itr = contents_.find(ck);
      if (itr == contents_.end()) {
        const auto i = vinit(k, ck);
        itr = contents_.insert({ck, {i,false}}).first;
        evict();
      }
      merge(cv, itr->second.cval);
      if (wt_) {
        db_set(ck, itr->second.cval);
      } 
      itr->second.is_dirty = !wt_;
    }

    friend void swap(Cache& lhs, Cache& rhs) {
      using std::swap;
      swap(lhs.db_, rhs.db_);
      swap(lhs.contents_, rhs.contents_);
      swap(lhs.wt_, rhs.wt_);
    }

  protected:
    // The default key
    virtual CKey kinit() {
      return CKey();
    }
    // The default val
    virtual CVal vinit() {
      return CVal();
    }
    // The default val to associate with a key
    virtual CVal vinit(const Key& k, const CKey& ck) {
      (void) k;
      (void) ck;
      return vinit();
    }

    // Map a user key to a cache key
    virtual CKey kmap(const Key& k) {
      return kinit();
    }
    // Map a user val to a cache val
    virtual CVal vmap(const Val& v) {
      return vinit();
    }
    // Merge a cache val with a pre-existing val
    virtual void merge(const CVal& v1, CVal& v2) {
      (void) v1;
      (void) v2;
    }
    // Invert the results of a cache lookup
    virtual Val vunmap(const Key& k, const CKey& ck, const CVal& cv) {
      (void) k;
      (void) ck;
      return cv;
    }

    // Serialize a cache key to a stream
    virtual void kwrite(std::ostream& os, const CKey& ck) {
      os << ck;
    }
    // Serialize a cache value to a stream
    virtual void vwrite(std::ostream& os, const CVal& cv) {
      os << cv;
    } 
    
    // Deserialize a cache key from a stream
    virtual void kread(std::istream& is, CKey& ck) {
      is >> ck;
    }
    // Deserialize a cache val from a stream
    virtual void vread(std::istream& is, CVal& cv) {
      is >> cv;
    }

  private:
    Database* db_;
    std::unordered_map<CKey, meta_type> contents_;
    bool wt_;

    bool db_get(const CKey& ck, CVal& cv) {
      std::stringstream ks;
      kwrite(ks, ck);
      const auto res = db_->get({ks.str().c_str(), ks.str().length()});
      std::stringstream vs({res.first.str, res.first.len}); 
      vread(vs, cv);
      return res.second;
    }
    void db_set(const CKey& ck, const CVal& cv) {
      std::stringstream ks;
      kwrite(ks, ck);
      std::stringstream vs;
      vwrite(vs, cv);
      db_->put({ks.str().c_str(), ks.str().length()}, {vs.str().c_str(), vs.str().length()});
    }

    void evict() {
      // TODO: Need to do something sensible here
    }
};

} // namespace binder

#endif
