# binder
A fuzzy database 

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

contains()
fetch()
flush()
get()
put()
