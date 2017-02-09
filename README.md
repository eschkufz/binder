# binder
A c++-14 stl-compliant implementation of a key-value store.

Dependencies
---
```
$ sudo apt-get install redis-server libhiredis-dev
```

fetch() is invoked whenever the key k is not found in s1. Should attempt to
find k in s2, and insert it into s1. Returns an interator into s1 on success,
or s1.end() on failure.

``` c++
template <typename S1, typename S2>
struct Fetch {
  typename S1::iterator fetch(S1& s1, S2& s2, const typename S1::k_type& k);
};
```

emplace() and insert() are invoked whenever a new key value pair is to be
inserted into s1. flush() is invoked whenever the pair at itr is about to be
evicted from s1 to write it to s2 if necessary.

``` c++
template <typename S1, typename S2>
struct Write {
  template <typename... Args>
  std::pair<typename S1::iterator, bool> emplace(S1& s1, S2& s2, Args&&... args);
  std::pair<typename S1::iterator, bool> insert(S1& s1, S2& s2, const typename S1::value_type& v);
  void flush(S2& s2, typename S1::const_iterator itr); 
};
```

touch() is invoked whenever <... when ...>. evict() is invoked whenever an item
is to be removed from s1. 

``` c++
template <typename S1>
struct Evict {
  void touch(typename S1::iterator itr);
  typename S1::const_iterator evict(S1& s1);
};
```

Map kmap()/vmap()/vunmap() are used to convert between types when moving
key-value pairs back and forth between primary store and backing store.

``` c++
template <typename DK, typename DV, 
          typename RK, typename RV>
struct Map {
  RK kmap(const DK& dk);
  RV vmap(const DV& dv);
  DV vunmap(const DK& dk, const RK& rk, const RV& rv);
};
``` 
