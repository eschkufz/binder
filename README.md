# binder
C++-14 stl-compliant key-value store containers.

Dependencies
---
```
$ sudo apt-get install redis-server libhiredis-dev
```

```Store``` is a simple in-memory key-value store. In addition to satisfying the interface and typedef requirements of an stl container, ```Store``` provides typedefs for querying key/value types and methods for modifying its contents. It is implemented in terms of an stl ```map```.

``` c++
template <typename Key, typename Value>
class Store {
  public:
    // stl container typedefs...
    // stl container interface...
  
    // store typedefs
    typedef const Key k_type;
    typedef const Value v_type;
    
    // store interface
    bool contains(const k_type& k);
    v_type get(const k_type& k);
    void put(const value_type& v);
    void erase(const k_type& k);
    void clear();
};
```

```UnorderedStore``` is defined equivalently, but is implemented in terms of an stl ```unordered_map```.
``` c++
template <typename Key, typename Value>
class UnorderedStore {
  public:
    // stl container typedefs...
    // stl container interface...
    // store typedefs...
    // store interface...
};
```
```RedisStore``` provides the same typedefs and interface as ```Store``` and ```UnorderedStore```, but is implemented in terms of a connection to a Redis key-value store. ```RedisStore``` also provides methods for opening and closing connections to Redis key-value stores and a convenience constructor.

Because Redis key-value stores store data as text, ```RedisStore``` takes a third template argument ```IO``` which represents a function object for converting ```Key``` and ```Value``` objects back and forth to and from text. If no object is provided ```RedisStore``` defaults to using the convenience class ```Stream``` which is defined in terms of the ```iostream``` insertion and extraction operators.

```c++
template <typename K, typename V>
struct IO {
  void kread(std::istream& is, K& k);
  void vread(std::istream& is, V& v);
  void kwrite(std::ostream& os, const K& k);
  void vwrite(std::ostream& os, const V& v);
};

template <typename K, typename V, typename IO=Stream<K,V>>
class RedisStore {
  public:
    // stl container typedefs...
    // stl container interface...
    // store typedefs...
    // store interface...    
    
    RedisStore(const string& host, unsigned int port);
    void connect(const string& host, unsigned int port);
    bool is_connected() const;
    void disconnect();
};
```

```c++
template <typename DK, typename DV, typename RK, typename RV>
struct Map {
  RK kmap(const DK& dk);
  RV vmap(const DV& dv);
  DK kunmap(const RK& rk);
  DV vunmap(const RV& rv);
  DV vunmap(const DK& dk, const RK& rk, const RV& rv);
};

template <typename K, typename V, typename S, typename M>
class AdapterStore {
  public:
    // stl container typedefs...
    // stl container interface...
    // store typedefs...
    // store interface...
    
    AdapterStore(S* backing_store);
    S* backing_store(S* s);
};
```

```c++
template <typename S>
struct Evict {
  void erase(const typename S::k_type& k);
  void touch(const typename S::k_type& k);
  typename S::k_type evict();
  friend void swap(Evict& lhs, Evict& rhs);
};

template <typename S>
struct Read {
  typedef /*...*/ const_iterator;

  void fetch(S& s, const typename S::k_type& k);
  const_iterator begin();
  const_iterator end();
  friend void swap(Read& lhs, Read& rhs);
};

template <typename S>
struct Write {
  void modify(S& s, const typename S::value_type& v);
  void flush(S& s, const typename S::k_type& k);
  friend void swap(Write& lhs, Write& rhs);
};

template <typename S1, typename S2,
          typename Evict, typename Read, typename Write>
class Cache {
  public:
    // stl container typedefs...
    // stl container interface...
    // store typedefs...
    // store interface... 
    
    Cache(S2* s2, size_t capacity);
    void set_capacity(size_t c);
    S2* backing_store(S2* s2);
};
```




