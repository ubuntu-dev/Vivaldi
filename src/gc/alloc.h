#ifndef VV_GC_ALLOC_H
#define VV_GC_ALLOC_H

#include "value/integer.h"

#include <array>

namespace vv {

namespace gc {

namespace internal {

value::object* get_next_empty(tag type);

template <typename T>
value::object* emplace(T&& item)
{
  auto slot = get_next_empty(tag_for<T>());
  new (slot) T{std::forward<T>(item)};
  return slot;
}

// Optimize common values
extern value::nil g_nil;
extern value::boolean g_true;
extern value::boolean g_false;
extern std::array<value::integer, 1024> g_ints;

}

template <typename T, typename... Args>
T* alloc(Args&&... args)
{
  static_assert(!std::is_same<T, value::boolean>(), "unspecialized for bool");
  static_assert(!std::is_same<T, value::nil>(),     "unspecialized for nil");
  static_assert(!std::is_same<T, value::integer>(), "unspecialized for integer");

  auto ptr = internal::emplace(T{std::forward<Args>(args)...});
  return static_cast<T*>(ptr);
}

// Optimized template overrides for alloc (warning: ugly) {{{

template <>
inline value::boolean* alloc<value::boolean, bool>(bool&& val)
{
  return &(val ? internal::g_true : internal::g_false);
}
template <>
inline value::boolean* alloc<value::boolean, bool&>(bool& val)
{
  return alloc<value::boolean, bool>(bool{val});
}
template <>
inline value::boolean* alloc<value::boolean>()
{
  return &internal::g_true;
}

template <>
inline value::nil* alloc<value::nil>()
{
  return &internal::g_nil;
}

template <>
inline value::integer* alloc<value::integer, int>(int&& val)
{
  if (val >= 0 && val < 1024)
    return &internal::g_ints[static_cast<unsigned>(val)];

  auto ptr = internal::emplace(value::integer{val});
  return static_cast<value::integer*>(ptr);
}

template <>
inline value::integer* alloc<value::integer, int&>(int& val)
{
  return alloc<value::integer, int>(int{val});
}

template <>
inline value::integer* alloc<value::integer>()
{
  return alloc<value::integer, int>(0);
}

// }}}

}

}

#endif
