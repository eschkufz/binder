#ifndef BINDER_INCLUDE_CACHE_H
#define BINDER_INCLUDE_CACHE_H

#include <string>
#include <unordered_map>

namespace binder {

/** 
  There are 3 layers of abstraction here:

  1. Key/Val       These are the types that are exposed to the user
  2. CKey/CVal     These are the types that are used by the cache
  3. string/string These are the types used by the database

  The cache is responsible for mapping between Key/Val and CKey/CVal
  CKey and CVal are responsible for supporting the following minimal interface:

  struct CKey {
    size_t hash() const;
    void read(const string& s);
    string write() const;
  };

  struct CVal {
    void read(const string& s);
    string write() const;
  };
*/

template <typename Key, typename Val, typename CKey, typename CVal>
class Cache {
  public:
    // Connect to a database at host/port
    Cache(const char* host, int port);
    virtual ~Cache();
    
    // Check connection status
    bool is_connected() const;

    // Methods for setting cache policies
    Cache& write_back(bool wb);
    Cache& write_through(bool wt);
    // ...

    // The basic interface
    bool contains(const Key& k);
    void put(const Key& k, const Val& v);
    Val get(const Key& k);

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
    // ...
};

} // namespace binder

#endif
