# binder
A fuzzy database 

Dependencies:
```
$ sudo apt-get install redis-server libhiredis-dev
```

This documentation is incomplete
---

There are 3 layers of abstraction here:
1. Key/Val       These are the types that are exposed to the user
2. CKey/CVal     These are the types that are used by the cache
3. string/string These are the types used by the database

```
template <typename Types, 
          typename WritePolicy, typename FetchPolicy, typename EvictPolicy,
          typename Storage>
class Cache {
  public:
    Cache(Database& db, size_t capacity);

    size_t size() const;
    size_t capacity() const;

    const_iterator begin() const;
    const_iterator end() const;

    void clear();
    bool contains(const Types::key_type& k);
    void fetch(const Types::key_type& k);
    void flush(const Types::key_type& k);
    Types::val_type get(const Types::key_type& k);
    void put(const Types::key_type& k, const Types::val_type& v);
};
```

```
struct MyTypes {
  // These typedefs must be provided
  typedef ... key_type;
  typedef ... val_type;
  typedef ... ckey_type;
  typedef ... cval_type;

  // Returns a ckey_type. No restrictions.
  static ckey_type kinit();
  // Returns a cval_type which guarantees that merge(v, vinit()) == v
  static cval_type vinit();
  // The object that should be returned when get(k) fails to find a match
  static cval_type vinit(const key_type& k, const ckey_type& ck);

  // Map a key_type to a ckey_type
  static ckey_type kmap(const key_type& k);
  // Map a val_type to a cval_type
  static cval_type vmap(const val_type& v);
  // Merge a cval_type with a pre-existing cval_type
  static void merge(const cval_type& v1, cval_type& v2);
  // Invert the results of a lookup
  static val_type vunmap(const key_type& k, const ckey_type& ck, const cval_type& cv);

  // Stream I/O
  static void kwrite(std::ostream& os, const ckey_type& k);
  static void vwrite(std::ostream& os, const cval_type& v);
  static void kread(std::istream& is, ckey_type& k);
  static void vread(std::istream& is, cval_type& v);

  // Transaction begin/end
  static void begin();
  static void end();
};
```

```
template <typename MyTypes, typename MyStorage>
struct FetchPolicy {
  // This typedef must be provided
  typedef ... const_iterator

  // A reference to the database connection 
  FetchPolicy(Database& db);

  // Invoked whenever a fetch is requested
  void fetch(const T::ckey_type& ck);

  // Iterators over fetch results.
  // If no results are returned, begin() == end() must hold.
  // If more than one result is returned, begin() must correspond
  // to the argument which was passed to fetch()
  const_iterator begin() const;
  const_iterator end() const;
};
```

```
template <typename MyTypes, typename MyStorage>
struct WritePolicy {
  // A reference to the database connection
  WritePolicy(Database& db);

  // Invoked whenever a cache entry is written
  void write(typename MyStorage::const_iterator line);
  // Invoked whenever the cache requests a sync
  void sync(typename MyStorage::const_iterator line);
};
```

``` 
template <typename MyTypes, typename MyStorage>
struct EvictPolicy {
  // Invoked whenever a cache entry is read or written
  void touch(typename MyStorage::const_iterator line);
  // Invoked whenever the cache requests an eviction
  typename MyStorage::const_iterator evict();
};
```
