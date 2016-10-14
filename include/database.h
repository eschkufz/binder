#ifndef BINDER_INCLUDE_DATABASE_H
#define BINDER_INCLUDE_DATABASE_H

#include <cassert>
#include <cstring>
#include <hiredis/hiredis.h>

namespace binder {

class Database {
  public:
    struct str_type {
      const char* str;
      size_t len;
    };
    typedef std::pair<str_type, str_type> line_type;

    class const_iterator : public std::iterator<std::forward_iterator_tag, line_type> {
      friend class Database;

      private:
        const_iterator(Database* db) : db_(db), scan_(nullptr) { }

      public:
        const_iterator() : db_(nullptr), scan_(nullptr) { }
        const_iterator(const const_iterator& rhs) : db_(rhs.db_), scan_(nullptr) {
          if (rhs.idx_ == -1) {
            end();
          } else {
            scan(rhs.ctok_, rhs.idx_);
          }
        }
        const_iterator(const_iterator&& rhs) : const_iterator() {
          swap(*this, rhs);
        }
        const_iterator& operator=(const_iterator rhs) {
          swap(*this, rhs);
          return *this;
        }
        ~const_iterator() {
          if (scan_ != nullptr) {
            freeReplyObject(scan_);
          }
        }

        const_iterator& operator++() {
          assert(scan_ != nullptr);
          if (++idx_ >= (int)scan_->element[1]->elements) {
            return ntok_ == 0 ? end() : scan(ntok_, 0);
          } 
          return *this;
        }
        const_iterator operator++(int) {
          const auto ret = *this;
          ++(*this);
          return ret;
        }

        bool operator==(const const_iterator& rhs) const {
          return ctok_ == rhs.ctok_ && ntok_ == rhs.ntok_ && idx_ == rhs.idx_;
        }
        bool operator!=(const const_iterator& rhs) const {
          return !(*this == rhs);
        }

        line_type operator*() const {
          assert(scan_ != nullptr);
          const auto ptr = scan_->element[1]->element[idx_];
          const str_type key = {ptr->str, (size_t)ptr->len};
          const auto res = db_->get(key);

          assert(res.second);
          return {key, res.first};
        }
        line_type* operator->() const {
          assert(false);
          return nullptr;
        }

        friend void swap(const_iterator& lhs, const_iterator& rhs) {
          using std::swap;
          swap(lhs.db_, rhs.db_);
          swap(lhs.scan_, rhs.scan_);
          swap(lhs.ctok_, rhs.ctok_);
          swap(lhs.ntok_, rhs.ntok_);
          swap(lhs.idx_, rhs.idx_);
        }

      private:
        Database* db_;
        redisReply* scan_;
        size_t ctok_;
        size_t ntok_;
        int idx_;

        const_iterator& end() {
          ctok_ = 0;
          ntok_ = 0;
          idx_ = -1;
          return *this;
        }
        const_iterator& scan(size_t ctok, int idx) {
          assert(db_->is_connected());
          if (scan_ != nullptr) {
            freeReplyObject(scan_);
          }

          scan_ = (redisReply*)redisCommand(db_->rc_, "SCAN %i", ctok);
          ctok_ = ctok;
          ntok_ = atoi(scan_->element[0]->str);
          idx_ = idx;
      
          if (idx_ >= (int)scan_->element[1]->elements) {
            return ntok_ == 0 ? end() : scan(ntok_, 0);
          }
          return *this;
        }
    };

    Database() : rc_(NULL), rep_(nullptr) {}
    Database(const Database& rhs) {
      connect(rhs.host_, rhs.port_);
    }
    Database(Database&& rhs) : Database() {
      swap(*this, rhs);
    }
    Database& operator=(Database rhs) {
      swap(*this, rhs);
      return *this;
    }
    ~Database() {
      disconnect();
    }

    Database& connect(const char* host, int port) {
      disconnect();

      host_ = host;
      port_ = port;
      rc_ = redisConnectWithTimeout(host_, port_, {1,500000});
      if (is_connected()) {
        rep_ = (redisReply*)redisCommand(rc_, "PING");
      }
      return *this;
    }
    Database& disconnect() {
      if (is_connected()) {
        redisFree(rc_);
      }
      if (rep_ != nullptr) {
        freeReplyObject(rep_);
      }
      return *this;
    }
    bool is_connected() const {
      return rc_ != NULL && !rc_->err;
    }

    size_t size() {
      assert(is_connected());
      assert(rep_ != nullptr);

      freeReplyObject(rep_);
      rep_ = (redisReply*)redisCommand(rc_, "DBSIZE");
      return rep_->integer;
    }
    const_iterator begin() const {
      return const_iterator(const_cast<Database*>(this)).scan(0,0);
    }
    const_iterator end() const {
      return const_iterator(const_cast<Database*>(this)).end();
    }

    void clear() {
      assert(is_connected());
      assert(rep_ != nullptr);

      freeReplyObject(rep_);
      rep_ = (redisReply*)redisCommand(rc_, "FLUSHDB");
    }
    bool contains(str_type k) {
      assert(is_connected());
      assert(rep_ != nullptr);

      freeReplyObject(rep_);
      rep_ = (redisReply*)redisCommand(rc_, "EXISTS %b", k.str, k.len);  
      return rep_->integer == 1;
    }
    std::pair<str_type, bool> get(str_type k) {
      assert(is_connected());
      assert(rep_ != nullptr);

      freeReplyObject(rep_);
      rep_ = (redisReply*)redisCommand(rc_, "GET %b", k.str, k.len);
      str_type v = {rep_->str, (size_t)rep_->len};
      return {v, strncmp(rep_->str, "(nil)", rep_->len)};
    }
    void put(str_type k, str_type v) {
      assert(is_connected());
      assert(rep_ != nullptr);

      freeReplyObject(rep_);
      rep_ = (redisReply*)redisCommand(rc_, "SET %b %b", k.str, k.len, v.str, v.len);
    }

    friend void swap(Database& lhs, Database& rhs) {
      using std::swap;
      swap(lhs.host_, rhs.host_);
      swap(lhs.port_, rhs.port_);
      swap(lhs.rc_, rhs.rc_);
      swap(lhs.rep_, rhs.rep_);
    }

  private:        
    const char* host_;
    int port_;
    redisContext* rc_;
    redisReply* rep_;
};

} // namespace binder

#endif
