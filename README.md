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

    bool is_connected() const;
    size_t size() const;
    line_type begin() const;
    line_type end() const;

    void clear();
    void flush();

    virtual bool contains(const Key& k);
    virtual void fetch(const Key& k);
    virtual void flush(const Key& k);
    virtual Val get(const Key& k);
    virtual void put(const Key& k, const Val& v);
};
```

Classes derived from Cache should support the following methods:
```
class MyCache : public<Key, Val, CKey, CVal> {
  protected:
    virtual CKey kinit();
    virtual CVal vinit(const Key& k, const CKey& ck);
    
    virtual CKey cmap(const Key& k) = 0;
    virtual CVal vmap(const Val& v) = 0;
    virtual void merge(const CVal& v1, CVal& v2) = 0;
    virtual Val vunmap(const Key& k, const CKey& ck, const CVal& cv) = 0;

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
