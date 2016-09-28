#ifndef BINDER_INCLUDE_BINDER_H
#define BINDER_INCLUDE_BINDER_H

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
    Cache() : rc_(NULL), wt_(false) {}
    Cache(const Cache& rhs) : cache_(rhs.cache_), wt_(rhs.wt_) {
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

    size_t size() const {
      return cache_.size();
    }
    bool is_connected() const {
      return rc_ != NULL && !rc_->err;
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

    const_iterator begin() const {
      return const_iterator(cache_.begin());
    }
    const_iterator end() const {
      return const_iterator(cache_.end());
    }

    void clear() {
      cache_.clear();
    }
    bool contains(const Key& k) {
      assert(is_connected());
      op_begin();

      const auto ck = cmap(k);
      const auto res = cache_.find(ck) != cache_.end() ? true : redis_exists(ck);

      op_end();
      return res;
    }
    void fetch(const Key& k) {
      assert(is_connected());
      op_begin();

      const auto ck = cmap(k);
      auto cv = init(k, ck);
      if (redis_get(ck, cv)) {
        cache_[ck] = {cv,false};
        evict();
      }

      op_end();
    }
    void flush(const Key& k) {
      assert(is_connected());
      op_begin();

      const auto ck = cmap(k);
      const auto itr = cache_.find(ck);
      if (itr != cache_.end() && itr->second.dirty) {
        redis_set(ck, itr->second.cval);
        itr->second.dirty = false;
      }

      op_end();
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
    Val get(const Key& k) {
      assert(is_connected());
      op_begin();

      const auto ck = cmap(k);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        auto cv = init(k, ck);
        redis_get(ck, cv);
        itr = cache_.insert({ck, {cv,false}}).first;
        evict();
      }
      const auto v = vunmap(k, ck, itr->second.cval);

      op_end();
      return v;
    }
    void put(const Key& k, const Val& v) {
      assert(is_connected());
      op_begin();

      const auto ck = cmap(k);
      const auto cv = vmap(v);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        const auto i = init(k, ck);
        itr = cache_.insert({ck, {i,false}}).first;
        evict();
      }
      merge(cv, itr->second.cval);
      if (wt_) {
        redis_set(ck, itr->second.cval);
      } 
      itr->second.dirty = !wt_;

      op_end();
    }

    friend void swap(Cache& lhs, Cache& rhs) {
      using std::swap;
      swap(lhs.rc_, rhs.rc_);
      swap(lhs.host_, rhs.host_);
      swap(lhs.port_, rhs.port_);
      swap(lhs.cache_, rhs.cache_);
      swap(lhs.wt_, rhs.wt_);
    }

  protected:
    // Optional state reset at the beginning of an operation
    virtual void op_begin() {}
    // Map a user key to a cache key
    virtual CKey cmap(const Key& k) = 0;
    // Map a user val to a cache val
    virtual CVal vmap(const Val& v) = 0;
    // Merge a cache val with a pre-existing val
    virtual void merge(const CVal& v1, CVal& v2) = 0;
    // The default val taht you would like to associate with a key
    virtual CVal init(const Key& k, const CKey& ck) = 0;
    // Invert the results of a cache lookup
    virtual Val vunmap(const Key& k, const CKey& ck, const CVal& cv) = 0;
    // Optional cleanup at the end of an operation
    virtual void op_end() {}

    // Serialize a cache key to a stream
    virtual void kwrite(std::ostream& os, const CKey& ck) {
      os << ck;
    }
    // Serialize a cache value to a stream
    virtual void vwrite(std::ostream& os, const CVal& cv) {
      os << cv;
    } 
    // Deserialize a cache val from a stream
    virtual void vread(std::istream& is, CVal& cv) {
      is >> cv;
    }

  private:
    redisContext* rc_;
    std::string host_;
    int port_;

    std::unordered_map<CKey, CacheLine> cache_;
    bool wt_;

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

    void evict() {
      // TODO: Need to do something sensible here
    }
};

} // namespace binder

#endif
