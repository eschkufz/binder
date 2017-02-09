#ifndef BINDER_INCLUDE_REDIS_H
#define BINDER_INCLUDE_REDIS_H

#include <hiredis/hiredis.h>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>
#include "ext/stl/include/buf_stream.h"

namespace binder {

template <typename K, typename V>
struct Stream {
  void kread(std::istream& is, K& k) {
    is >> k;
  }
  void vread(std::istream& is, V& v) {
    is >> v;
  }
  void kwrite(std::ostream& os, const K& k) {
    os << k;
  }
  void vwrite(std::ostream& os, const V& v) {
    os << v;
  }
};

template <typename K, typename V, typename IO = Stream<K,V>>
class RedisStore {
  public:
    template <bool is_const>
    class Iterator {
      friend class RedisStore;

      // TYPES:
      private:
        typedef typename RedisStore::reference ref;
        typedef typename RedisStore::value_type* ptr;
      public:
        typedef typename RedisStore::value_type value_type;
        typedef typename std::conditional<is_const, const ref, ref>::type reference;
        typedef typename std::conditional<is_const, const ptr, ptr>::type pointer;
        typedef typename RedisStore::difference_type difference_type;
        typedef typename std::forward_iterator_tag iterator_category;

      // CONSTRUCT/COPY/DESTROY:
      private:
        Iterator(RedisStore* rs) : rs_(rs), scan_(nullptr), val_(nullptr) { }
      public:
        Iterator() : rs_(nullptr), scan_(nullptr), val_(nullptr) { }
        Iterator(const Iterator& rhs) : rs_(rhs.rs_), scan_(nullptr), val_(nullptr) {
          scan(rhs.cursor_, rhs.idx_); 
        }
        Iterator(Iterator&& rhs) : Iterator() {
          swap(rhs);
        }
        Iterator& operator=(Iterator rhs) {
          swap(rhs);
          return *this;
        }
        ~Iterator() {
          if (scan_ != nullptr) {
            freeReplyObject(scan_);
          }
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
          if (scan_ == nullptr) {
            scan(0, 0);
            return *this;
          } 
          ++idx_;
          while (idx_ == (size_t)scan_->element[1]->elements) {
            const auto ncursor = atoi(scan_->element[0]->str);
            if (ncursor == 0) {
              freeReplyObject(scan_);
              scan_ = nullptr;
              return *this;
            } 
            scan(ncursor, 0);
          }
          return *this;
        }
        Iterator operator++(int) {
          auto ret = *this;
          ++(*this);
          return ret;
        }
        bool operator==(const Iterator& rhs) const {
          return (scan_ == nullptr && rhs.scan_ == nullptr) ||
              (cursor_ == rhs.cursor_ && idx_ == rhs.idx_);
        }
        bool operator!=(const Iterator& rhs) const {
          return !(*this == rhs);
        }

      private:
        RedisStore* rs_;
        redisReply* scan_;
        value_type* val_;
        unsigned int cursor_;
        size_t idx_;

        void scan(unsigned int cursor, size_t idx) {
          if (scan_ != nullptr) {
            freeReplyObject(scan_);
          }
          if (rs_ == nullptr || !rs_->is_connected()) {
            scan_ = nullptr;
          } else {
            scan_ = (redisReply*)redisCommand(rs_->rc_, "SCAN %i", cursor); 
            cursor_ = cursor;
            idx_ = idx >= (size_t)scan_->element[1]->elements ? 0 : idx;
          }
        }
        void get() {
          if (val_ != nullptr) {
            delete val_;
          }
          if (rs_ == nullptr || !rs_->is_connected() || scan_ == nullptr) {
            val_ = new value_type();
          } else {
            auto ptr = scan_->element[1]->element[idx_];
            stl::buf_stream bs(ptr->str, ptr->str+ptr->len);
            K k;
            IO().kread(bs, k);
            const auto v = rs_->get(ptr->str, (size_t)ptr->len);
            val_ = new value_type(std::move(k), std::move(v));
          }
        }
        void swap(Iterator& rhs) {
          using std::swap;
          swap(rs_, rhs.rs_);
          swap(scan_, rhs.scan_);
          swap(cursor_, rhs.cursor_);
          swap(idx_, rhs.idx_);
          swap(val_, rhs.val_);
        }
    };

    // TYPES:
    // Container:
    typedef std::pair<const K, const V> value_type;
    typedef value_type& reference;
    typedef const reference const_reference;
    typedef Iterator<false> iterator;
    typedef Iterator<true> const_iterator;
    typedef ptrdiff_t difference_type;
    typedef size_t size_type;
    // Other:
    typedef const K k_type;
    typedef const V v_type;

    // CONSTRUCT/COPY/DESTROY:
    // Container:
    RedisStore() : rc_(NULL) { }
    RedisStore(const RedisStore& rhs) : rc_(NULL), host_(rhs.host_), port_(rhs.port_) {
      if (rhs.is_connected()) {
        connect(host_, port_);
      }
    }
    RedisStore(RedisStore&& rhs) : RedisStore() {
      swap(*this, rhs);
    }
    RedisStore& operator=(RedisStore rhs) {
      swap(*this, rhs);
      return *this;
    }
    ~RedisStore() {
      disconnect();
    }
    // RedisStore:
    RedisStore(const std::string& host, unsigned int port) {
      connect(host, port);
    }

    // ITERATORS:
    // Container:
    iterator begin() {
      return ++iterator(this);
    }
    const_iterator begin() const {
      return ++const_iterator(const_cast<RedisStore*>(this));
    }
    iterator end() {
      return iterator(this);
    }
    const_iterator end() const {
      return iterator(const_cast<RedisStore*>(this));
    }
    const_iterator cbegin() const {
      return ++const_iterator(const_cast<RedisStore*>(this));
    }
    const_iterator cend() const {
      return const_iterator(const_cast<RedisStore*>(this));
    }

    // CAPACITY:
    // Container:
    bool empty() const {
      return size() == 0;
    }
    size_type size() const {
      if (!is_connected()) {
        return 0;
      }

      auto rep = (redisReply*)redisCommand(rc_, "DBSIZE");
      const auto res = rep->integer;
      freeReplyObject(rep);

      return res;
    }
    size_type max_size() const {
      return std::numeric_limits<size_type>::max();
    }

    // MODIFIERS:
    // Container:
    void swap(RedisStore& rhs) {
      using std::swap;
      swap(host_, rhs.host_);
      swap(port_, rhs.port_);
      swap(rc_, rhs.rc_);
    }

    // STORE INTERFACE:
    // Common:
    bool contains(const k_type& k) {
      if (!is_connected()) {
        return false;
      }

      std::stringstream kss;
      IO().kwrite(kss, k);
      auto rep = (redisReply*)redisCommand(rc_, "EXISTS %b", kss.str().c_str(), kss.str().length());
      const auto res = rep->integer == 1;
      freeReplyObject(rep);

      return res;
    }
    v_type get(const k_type& k) {
      if (!is_connected()) {
        return v_type();
      }

      std::stringstream kss;
      IO().kwrite(kss, k);
      return get(kss.str().c_str(), kss.str().length());
    }
    void put(const value_type& v) {
      if (!is_connected()) {
        return;
      }

      std::stringstream kss;
      IO().kwrite(kss, v.first);
      std::stringstream vss;
      IO().vwrite(vss, v.second);

      auto rep = (redisReply*)redisCommand(rc_, "SET %b %b",
          kss.str().c_str(), kss.str().length(), vss.str().c_str(), vss.str().length());
      freeReplyObject(rep);
    }
    void erase(const k_type& k) {
      if (!is_connected()) {
        return;
      }

      std::stringstream kss;
      IO().kwrite(kss, k);
      auto rep = (redisReply*)redisCommand(rc_, "DEL %b", kss.str().c_str(), kss.str().length());
      freeReplyObject(rep);
    }
    void clear() {
      if (!is_connected()) {
        return;
      }
      auto rep = (redisReply*)redisCommand(rc_, "FLUSHDB");
      freeReplyObject(rep);
    }
    // RedisStore:
    void connect(const std::string& host, unsigned int port) {
      if (is_connected()) {
        disconnect();
      }
      host_ = host;
      port_ = port;
      rc_ = redisConnectWithTimeout(host.c_str(), port, {1,500000});
    }
    bool is_connected() const {
      return rc_ != NULL && !rc_->err;
    }
    void disconnect() {
      if (rc_ != NULL) {
        redisFree(rc_);
        rc_ = NULL;
      } 
    }

    // COMPARISON:
    // Container:
    friend bool operator==(const RedisStore& lhs, const RedisStore& rhs) {
      return lhs.host_ == rhs.host_ && lhs.port_ == rhs.port_;
    }
    friend bool operator!=(const RedisStore& lhs, const RedisStore& rhs) {
      return !(lhs == rhs);
    }

    // SPECIALIZED ALGORITHMS:
    // Container:
    friend void swap(RedisStore& lhs, RedisStore& rhs) {
      lhs.swap(rhs);
    }

  private:
    std::string host_;
    unsigned int port_;
    redisContext* rc_;

    v_type get(const char* k, size_t len) {
      V v;
      auto rep = (redisReply*)redisCommand(rc_, "GET %b", k, len);
      stl::buf_stream bs(rep->str, rep->str+rep->len);
      IO().vread(bs, v);
      freeReplyObject(rep);
      return v;
    }
};

} // namespace binder

#endif
