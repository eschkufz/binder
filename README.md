# binder
C++-14 stl-compliant key-value store containers.

Dependencies
---
```
$ sudo apt-get install redis-server libhiredis-dev
```

Documentation
---
```Store``` is a simple in-memory key-value store. In addition to satisfying
the interface and typedef requirements of an stl container, ```Store```
provides typedefs for compile-time querying the ```Key``` and ```Value```
types, and runtime methods for modifying its contents. The semantics of these
methods are suggested by their names. The only non-obvious behavior is that
invoking ```get()``` for a non-existent key will return a default constructed
```Value```. ```Store``` is implemented in terms of an stl ```map```.

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

```UnorderedStore``` is defined equivalently, but is implemented in terms of an
stl ```unordered_map```.
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
```RedisStore``` provides the same typedefs and interface as ```Store``` and
```UnorderedStore```, but is implemented in terms of a connection to a Redis
key-value store. ```RedisStore``` also provides methods for opening and closing
connections to Redis key-value stores and a convenience constructor.

Because Redis key-value stores store data as text, ```RedisStore``` takes a
third template argument ```IO``` which represents a function object for
converting ```Key``` and ```Value``` objects back and forth to and from text.
If no object is provided ```RedisStore``` defaults to using the convenience
class ```Stream``` which is defined in terms of the ```iostream``` insertion
and extraction operators.

```c++
template <typename Key, typename Value>
struct IO {
  void kread(std::istream& is, Key& k);
  void vread(std::istream& is, Value& v);
  void kwrite(std::ostream& os, const Key& k);
  void vwrite(std::ostream& os, const Value& v);
};

template <typename Key, typename Value, typename IO=Stream<Key,Value>>
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

In some cases, it may be useful to treat a store ```S``` for types ```RKey```
and ```RValue``` as though it were defined in terms of (potentially) different
types ```DKey``` and ```DValue```. This functionality is provided by the class
```Adapter``` which is parameterized by a backing store ```S``` and a function
object ```Map``` for converting back and forth between objects of type
```Dkey``` and ```RKey``` and objects of type ```DValue``` and ```RValue```.
The ```Adapter``` class also provides a convenience constructor and a method
for swapping out the backing store. 

In addition to providing type abstraction, the ```Map``` class can also provide
a simple form of value abstraction. Whereas iterator dereference will invoke
the unary forms of ```kunmap()``` and ```vunmap()```, the ```get()``` method
will invoke the ternary form which which is aware of both the ```DKey``` and
```RKey``` which correspond to the returned value. This allows the user to
define mappings which represent invertible onto relationships. If no mapping
object is provided, binder defaults to using the convenience class ```Cast```
which is defined in terms of C-style casts between types.

```c++
template <typename DKey, typename DValue, typename RKey, typename RValue>
struct Map {
  RKey kmap(const DKey& dk);
  RValue vmap(const DValue& dv);
  DKey kunmap(const RKey& rk);
  DValue vunmap(const RValue& rv);
  DValue vunmap(const DKey& dk, const RKey& rk, const RValue& rv);
};

template <typename Key, typename Value, typename S, 
          typename Map=Cast<Key, Value, S::k_type, S::v_type>>
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

In some cases it may also be useful to use one store as a cache for another.
This functionality is provided by the ```Cache``` class which is defined in
terms of a primary store ```S1``` and a backing-store ```S2``` as well as
function objects which represent policies for ```Evict```-ing data from
```S1```, ```Read```-ing data from ```S2``` into ```S1``` and ```Write```-ing
data from ```S1```  into ```S2```. The ```Cache``` class also provides a
convenience constructor and methods for modifying its capacity and swapping out
its backing store. 

The interface for the three policy objects is shown below. ```Evict::erase()```
is invoked whenever a key is removed from ```S1``` and ```Evict::touch()``` is
invoked whenever a key is get or set in ```S1```. ```Evict::evict()``` is used
to select a key from ```S1``` which should be written back to ```S2``` when
```S1``` is full. ```Read::fetch()``` is invoked whenever a key cannot be
located in ```S1``` and guarantees that ```Read::begin()``` and
```Read::end()``` can be used to iterate over the data which should be moved
into ```S1``` as a result. ```Write::modify()``` is invoked at the first
possible moment when data is put into ```S1``` and might also need to be put
into ```S2```, and ```Write::flush()``` is invoked at the last possible moment.
All three policies also require an stl-style ```swap()``` method.

binder provides a ```Lru``` evict policy, a ```Fetch``` read policy, and
```WriteBack``` and ```WriteThrough``` write policies.

```c++
template <typename S1>
struct Evict {
  void erase(const typename S1::k_type& k);
  void touch(const typename S1::k_type& k);
  typename S1::k_type evict();
  friend void swap(Evict& lhs, Evict& rhs);
};

template <typename S2>
struct Read {
  typedef /*...*/ const_iterator;

  void fetch(S2& s, const typename S2::k_type& k);
  const_iterator begin();
  const_iterator end();
  friend void swap(Read& lhs, Read& rhs);
};

template <typename S2>
struct Write {
  void modify(S2& s, const typename S2::value_type& v);
  void flush(S2& s, const typename S2::k_type& k);
  friend void swap(Write& lhs, Write& rhs);
};

template <typename S1, typename S2,
          typename Evict=Lru<S1>, 
          typename Read=Fetch<S2>, 
          typename Write=WriteThrough<S2>>
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

Usage
---
```c++
#include "include/binder.h"
using namespace binder;
using namespace std;

int main() {
  // A simple store
  Store<int, char> s1;
  s1.put(make_pair(1,'a'));
  s1.get(1);
  
  // A simple unordered store
  UnorderedStore<int, char> s2;
  for (const auto& p : s1) {
    s2.put(p);
  }
  if (s2.contains(1)) {
    s2.erase(1);
  }
  s2.clear();

  // A redis-backed store
  RedisStore<double, long> s3("localhost", 6379);
  
  // Change the exposed types of s2
  AdapterStore<double, long, decltype(s2)> s4(&2);
  
  // Now that the types match, use s4 as a cache for s3
  Cache<decltype(s4), decltype(s3)> s5(&s4, &s3);

  // Do anything with s5 that you would do with an stl container!
  cout << std::max_element(s5.begin(), s5.end()).first << endl;
  
  return 0;
}
```
