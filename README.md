# binder
A fuzzy database 

Dependencies:
```
$ sudo apt-get install redis-server libhiredis-dev
```

There are 3 layers of abstraction here:
1. Key/Val       These are the types that are exposed to the user
2. CKey/CVal     These are the types that are used by the cache
3. string/string These are the types used by the database

The cache supports the following basic interface:
```
template <typename Key, typename Val, typename CKey, typename CVal>
class Cache {
  public:
    Cache();
    Cache(const Cache& rhs);
    Cache(Cache&& rhs);
    virtual ~Cache();
    
    Cache& connect(const std::string& host, int port);
    Cache& write_through(bool wt);

    size_t size() const;
    bool is_connected() const;

    line_type begin() const;
    line_type end() const;

    void clear();
    bool contains(const Key& k);
    void fetch(const Key& k);
    void flush(const Key& k);
    void flush();
    Val get(const Key& k);
    void put(const Key& k, const Val& v);
};
```

Classes derived from Cache should support the following methods:
```
class MyCache : public<Key, Val, CKey, CVal> {
  protected:
    virtual void op_begin();
    virtual CKey cmap(const Key& k);
    virtual CVal vmap(const Val& v);
    virtual void merge(const CVal& v1, CVal& v2);
    virtual CVal init(const Key& k, const CKey& ck);
    virtual Val vunmap(const Key& k, const CKey& ck, const CVal& cv);
    virtual void op_end();

    virtual void cwrite(ostream& os, const CKey& ck);
    virtual void vwrite(ostream& os, const CVal& cv);
    virtual void vread(istream& is, CVal& cv);
};
```
Additionally, the following method must be defined:
```
namespace std {
template <>
struct hash<CKey> {
  size_t operator()(const CKey& ck) const;
};
}
