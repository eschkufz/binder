#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include <sstream>
#include <unordered_map>

#include "include/database.h"

template <typename Key, typename Val, typename CKey, typename CVal>
class Cache {
  public:
    Cache(Database* db) : db_(db), wt_(false) { }
    Cache(const Cache& rhs) : db_(rhs.db_), contents_(rhs.contents_), wt_(rhs.wt_) { }
    Cache(Cache&& rhs) : Cache() {
      swap(*this, rhs);
    }    
    Cache& operator=(Cache rhs) {
      swap(*this, rhs);
      return *this;
    }
    virtual ~Cache() { 
      if (db_->is_connected()) {
        flush();
      }
    }

    Cache& write_through(bool wt) {
      wt_ = wt;
      return *this;
    }

    size_t size() const {
      return contents_.size();
    }
    // ...

    void clear() {
      contents_.clear();
    }
    void fetch() {
      // ...
    }
    void flush() {
      for (auto& line : contents_) {
        if (line.second.dirty) {
          db_set(line.first, line.second.cval);
          line.second.dirty = false;
        }
      }
    }

    virtual bool contains(const Key& k) {
      const auto ck = kmap(k);
      return cache_.find(ck) != cache_.end() ? true : db_contains(ck);
    }
    virtual void fetch(const Key& k) {
      const auto ck = kmap(k);
      auto cv = vinit(k, ck);
      if (db_get(ck, cv)) {
        cache_[ck] = {cv,false};
        evict();
      }
    }
    virtual void flush(const Key& k) {
      const auto ck = kmap(k);
      const auto itr = cache_.find(ck);
      if (itr != cache_.end() && itr->second.dirty) {
        db_set(ck, itr->second.cval);
        itr->second.dirty = false;
      }
    }
    virtual Val get(const Key& k) {
      const auto ck = kmap(k);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        auto cv = vinit(k, ck);
        db_get(ck, cv);
        itr = cache_.insert({ck, {cv,false}}).first;
        evict();
      }

      const auto v = vunmap(k, ck, itr->second.cval);
      return v;
    }
    virtual void put(const Key& k, const Val& v) {
      const auto ck = kmap(k);
      const auto cv = vmap(v);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        const auto i = vinit(k, ck);
        itr = cache_.insert({ck, {i,false}}).first;
        evict();
      }
      merge(cv, itr->second.cval);
      if (wt_) {
        db_set(ck, itr->second.cval);
      } 
      itr->second.dirty = !wt_;
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
      return CVal();
    }

    // Map a user key to a cache key
    virtual CKey kmap(const Key& k) = 0;
    // Map a user val to a cache val
    virtual CVal vmap(const Val& v) = 0;
    
    // Merge a cache val with a pre-existing val
    virtual void merge(const CVal& v1, CVal& v2) = 0;
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
    std::unordered_map<CKey, val_type> contents_;
    bool wt_;

    bool db_contains(const CKey& ck) {
      std::stringstream ks;
      kwrite(ks, ck);
      return db_->contains({ks.str().c_str(), ks.str().length()});
    }
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

#endif
