#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include <cassert>
#include <hiredis/hiredis.h>
#include <string>
#include <unordered_map>

namespace binder {

template <typename Key, typename Val, typename CKey, typename CVal>
class Cache {
  public:
    Cache(const char* host, int port) : wt_(false) {
      rc_ = redisConnectWithTimeout(host, port, {1,500000});
    }
    virtual ~Cache() {
      if (is_connected()) {
        flush();
        redisFree(rc_);
      }
    }
    
    bool is_connected() const {
      return rc_ != NULL && !rc_->err;
    }

    Cache& write_through(bool wt) {
      wt_ = wt;
      return *this;
    }

    bool contains(const Key& k) {
      assert(is_connected());
      begin();

      const auto ck = cmap(k);
      const auto res = (cache_.find(ck) != cache_.end()) : true : redis_exists(ck);

      end();
      return res;
    }
    void fetch(const Key& k) {
      assert(is_connected());
      begin();

      const auto ck = cmap(k);
      auto cv = init(k, ck);
      if (redis_get(ck, cv)) {
        cache_[ck] = {cv,false};
        evict();
      }

      end();
    }
    void flush(const Key& k) {
      assert(is_connected());
      begin();

      const auto ck = cmap(k);
      const auto itr = cache_.find(ck);
      if (itr != cache_.end() && itr->second.dirty) {
        redis_set(ck, itr->second.cval);
        itr->second.dirty = false;
      }

      end();
    }
    void flush() {
      assert(is_connected());
      for (const auto& line : cache_) {
        if (line.second.dirty) {
          redis_set(line.first, line.second.cval);
          line.second.dirty = false;
        }
      }
    }
    Val get(const Key& k) {
      assert(is_connected());
      begin();

      const auto ck = cmap(k);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        auto cv = init(k, ck);
        redis_get(ck, cv);
        itr = cache_.insert(std::make_pair(ck, {cv,false})).first;
        evict();
      }
      const auto v = vunmap(itr->second.cval);

      end();
      return v;
    }
    void put(const Key& k, const Val& v) {
      assert(is_connected();
      begin();

      const auto ck = cmap(k);
      const auto cv = vmap(v);
      auto itr = cache_.find(ck);
      if (itr == cache_.end()) {
        const auto i = init(k, ck);
        itr = cache_.insert(std::make_pair(ck, {i,false})).first;
        evict();
      }
      merge(cv, itr->second.cval);
      if (wt_) {
        redis_set(cv, itr->second.cval);
      } 
      itr->second.dirty = !wt_;

      end();
    }

  protected:
    // Optional state reset at the beginning of an operation
    virtual void begin() = 0;
    // Map a user key to a cache key
    virtual CKey cmap(const Key& k) = 0;
    // Map a user val to a cache val
    virtual CVal vmap(const Val& v) = 0;
    // Merge a cache val with a pre-existing val
    virtual void merge(const CVal& v1, Cval& v2) = 0;
    // The default val taht you would like to associate with a key
    virtual CVal init(const Key& k, const CKey& ck) = 0;
    // Invert the results of a cache lookup
    virtual Val vunmap(const Key& k, const CKey& ck, const CVal& cv) = 0;
    // Optional cleanup at the end of an operation
    virtual void end() = 0;

  private:
    struct Line {
      CVal cval;
      bool dirty;
    };
    struct Hash {
      size_t operator()(const CKey& ck) const {
        return ck.hash();
      }
    };
    struct Equal {
      bool operator()(const CKey& ck1, const CKey& ck2) const {
        return ck1.equals(ck2);
      }
    };

    std::unordered_map<CKey, Line, Hash, Equal> cache_;
    redisContext* rc_;
    bool wt_;

    bool redis_exists(const CKey& ck) {
      const auto ks = ck.write();
      const auto rep = (redisReply*)redisCommand(rc_, "EXISTS %b", ks.c_str(), ks.length());  
      const auto res = rep->integer == 1;
      freeReplyObject(rep);
      return res;
    }
    bool redis_get(const CKey& ck, CVal& cv) {
      const auto ks = ck.write();
      const auto rep = (redisReply*)redisCommand(rc_, "GET %b", ks.c_str(), ks.length());
      const std::string vs(rep->str, rep->len);

      if (vs == "(nil)") {
        return false;
      }
      cv.read(vs);
      return true;
    }
    void redis_set(const CKey& ck, const CVal& cv) {
      const auto ks = ck.write();
      const auto vs = cv.write();
      const auto rep = (redisReply*)
        redisCommand(rc_, "SET %b %b", ks.c_str(), ks.length(), vs.c_str(), vs.length());
      freeReplyObject(rep);
    }

    void evict() {
      // TODO: Need to do something sensible here
    }
};

} // namespace binder

#endif
