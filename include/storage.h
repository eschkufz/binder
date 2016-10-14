#ifndef BINDER_INCLUDE_STORAGE_H
#define BINDER_INCLUDE_STORAGE_H

#include <map>

namespace binder {

template <typename T>
using Storage = std::map<typename T::ckey_type, typename T::cval_type>;

} // namespace binder

#endif
