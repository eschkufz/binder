#ifndef BINDER_INCLUDE_BINDER_H
#define BINDER_INCLUDE_BINDER_H

#include "include/cache.h"
#include "include/database.h"

#include <cassert>
#include <hiredis/hiredis.h>
#include <iterator>
#include <sstream>
#include <string>
#include <unordered_map>

namespace binder {

template <typename Key, typename Val, typename CKey, typename CVal>
class Cache {
  private:
    struct CacheLine {
      CVal cval;
      bool dirty;
    };

  public:
    Cache() : rc_(NULL), wt_(false), scan_(nullptr) {}
    Cache(const Cache& rhs) : cache_(rhs.cache_), wt_(rhs.wt_), scan_(nullptr) {
      connect(rhs.host_, rhs.port_); 
    }
    Cache(Cache&& rhs) : Cache() {
      swap(*this, rhs);
    }
    virtual ~Cache() {
      if (is_connected()) {
        flush();
        redisFree(rc_);
      }
      if (scan_ != nullptr) {
        freeReplyObject(scan_);
      }
    }
    
    Cache& connect(const std::string& host, int port) {
      assert(!is_connected());
      host_ = host;
      port_ = port;
      rc_ = redisConnectWithTimeout(host_.c_str(), port_, {1,500000});
      return *this;
    }
    Cache& write_through(bool wt) {
      wt_ = wt;
      return *this;
    }

    typedef std::pair<const CKey&, const CVal&> line_type;
    class const_iterator : public std::iterator<std::forward_iterator_tag, line_type> {
      friend class Cache;
      private:
        const_iterator(typename std::unordered_map<CKey, CacheLine>::const_iterator itr) : itr_(itr) { }

      public:
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
          return itr_ != rhs.itr_;
        }

        line_type operator*() const {
          return {itr_->first, itr_->second.cval};
        }
        line_type* operator->() const {
          return nullptr;
        }
      
      private: 
        typename std::unordered_map<CKey, CacheLine>::const_iterator itr_;
    };

    bool is_connected() const {
      return rc_ != NULL && !rc_->err;
    }
    size_t size() const {
      return cache_.size();
    }
    const_iterator begin() const {
      return const_iterator(cache_.begin());
    }
    const_iterator end() const {
      return const_iterator(cache_.end());
    }

    void clear() {
      cache_.clear();
    }
    void fetch() {
      assert(is_connected());

      auto ck = kinit();
      for (redis_scan(); redis_scan(ck);) {
        if (cache_.find(ck) == cache_.end()) {
          auto cv = vinit();
          redis_get(ck, cv);
          cache_.insert({ck, {cv,false}});
          evict();
        }
      }
    }
    void flush() {
      assert(is_connected());

      for (auto& line : cache_) {
        if (line.second.dirty) {
          redis_set(line.first, line.second.cval);
          line.second.dirty = false;
        }
      }
    }

    virtual bool contains(const Key& k) {
      assert(is_connected());

      const auto ck = kmap(k);
      const auto res = cache_.find(ck) != cache_.end() ? true : redis_exists(ck);

      return res;
    }
    virtual void fetch(const Key& k) {
      assert(is_connected());

      const auto ck = kmap(k);
      auto cv = vinit(k, ck);
      if (redis_get(ck, cv)) {
        cache_[ck] = {cv,false};
        evict();
      }
    }
    virtual void flush(const Key& k) {
      assert(is_connected());

      const auto ck = kmap(k);
      const auto itr = cache_.find(ck);
      if (itr != cache_.end() && itr->second.dirty) {
        redis_set(ck, itr->second.cval);
        itr->second.dirty = false;
      }
    }
    virtual Val get(const Key& k) {
      assert(is_connected());

      const auto ck = kmap(k);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        auto cv = vinit(k, ck);
        redis_get(ck, cv);
        itr = cache_.insert({ck, {cv,false}}).first;
        evict();
      }

      const auto v = vunmap(k, ck, itr->second.cval);
      return v;
    }
    virtual void put(const Key& k, const Val& v) {
      assert(is_connected());

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
        redis_set(ck, itr->second.cval);
      } 
      itr->second.dirty = !wt_;
    }

    friend void swap(Cache& lhs, Cache& rhs) {
      using std::swap;
      swap(lhs.rc_, rhs.rc_);
      swap(lhs.host_, rhs.host_);
      swap(lhs.port_, rhs.port_);
      swap(lhs.cache_, rhs.cache_);
      swap(lhs.wt_, rhs.wt_);
      swap(lhs.scan_, rhs.scan_);
      swap(lhs.idx_, rhs.idx_);
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
    // Connection state
    redisContext* rc_;
    std::string host_;
    int port_;

    // Cache state
    std::unordered_map<CKey, CacheLine> cache_;
    bool wt_;

    // Scan state
    redisReply* scan_;
    size_t idx_;

    bool redis_exists(const CKey& ck) {
      std::stringstream ks;
      kwrite(ks, ck);
      const auto rep = (redisReply*)redisCommand(rc_, "EXISTS %b", ks.str().c_str(), ks.str().length());  
      const auto res = rep->integer == 1;
      freeReplyObject(rep);
      return res;
    }
    bool redis_get(const CKey& ck, CVal& cv) {
      std::stringstream ks;
      kwrite(ks, ck);
      const auto rep = (redisReply*)redisCommand(rc_, "GET %b", ks.str().c_str(), ks.str().length());
      std::stringstream vs({rep->str, (size_t)rep->len}); 
      if (vs.str() == "(nil)") {
        return false;
      }
      vread(vs, cv);
      freeReplyObject(rep);
      return true;
    }
    void redis_set(const CKey& ck, const CVal& cv) {
      std::stringstream ks;
      kwrite(ks, ck);
      std::stringstream vs;
      vwrite(vs, cv);
      const auto rep = (redisReply*)
        redisCommand(rc_, "SET %b %b", ks.str().c_str(), ks.str().length(), vs.str().c_str(), vs.str().length());
      freeReplyObject(rep);
    }
    void redis_scan() {
      if (scan_ != nullptr) {
        freeReplyObject(scan_);
      }
      scan_ = (redisReply*)redisCommand(rc_, "SCAN 0");
      idx_ = 0;
    }
    bool redis_scan(CKey& ck) {
      assert(scan_ != nullptr);

      if (idx_ == scan_->element[1]->elements) {
        const size_t token = atoi(scan_->element[0]->str);
        if (token == 0) {
          return false;
        }
        freeReplyObject(scan_);
        scan_ = (redisReply*)redisCommand(rc_, "SCAN %i", token);
        idx_ = 0;
      }

      const auto ptr = scan_->element[1]->element[idx_++];
      std::stringstream ks({ptr->str, (size_t)ptr->len});
      kread(ks, ck);
      return true;
    }

    void evict() {
      // TODO: Need to do something sensible here
    }
};

} // namespace binder

#endif
