#ifndef VV_GC_ALLOC_H
#define VV_GC_ALLOC_H

#include "value.h"
#include "gc/managed_ptr.h"

namespace vv {

namespace gc {

namespace internal {

managed_ptr get_next_empty(tag type);

template <typename T>
managed_ptr emplace(T&& item)
{
  auto slot = get_next_empty(tag_for<T>());
  new (slot.get()) T{std::forward<T>(item)};
  return slot;
}

}

template <typename T, typename... Args>
gc::managed_ptr alloc(Args&&... args)
{
  static_assert(!std::is_same<T, value::boolean>(),   "unspecialized for bool");
  static_assert(!std::is_same<T, value::character>(), "unspecialized for char");
  static_assert(!std::is_same<T, value::nil>(),       "unspecialized for nil");
  static_assert(!std::is_same<T, value::integer>(),   "unspecialized for integer");

  auto ptr = internal::emplace(T{std::forward<Args>(args)...});
  return ptr;
}

// Optimized template overrides for alloc (warning: ugly) {{{

template <>
inline gc::managed_ptr alloc<value::boolean, bool>(bool&& val)
{
  return {val, 0, tag::boolean, 1};
}
template <>
inline gc::managed_ptr alloc<value::boolean, bool&>(bool& val)
{
  return {val, 0, tag::boolean, 1};
}
template <>
inline gc::managed_ptr alloc<value::boolean, const bool&>(const bool& val)
{
  return {val, 0, tag::boolean, 1};
}
template <>
inline gc::managed_ptr alloc<value::boolean>()
{
  return {1, 0, tag::boolean, 1};
}

template <>
inline gc::managed_ptr alloc<value::character, char>(char&& val)
{
  return {static_cast<uint32_t>(val), 0, tag::character, 1};
}

template <>
inline gc::managed_ptr alloc<value::character, char&>(char& val)
{
  return {static_cast<uint32_t>(val), 0, tag::character, 1};
}

template <>
inline gc::managed_ptr alloc<value::character, const char&>(const char& val)
{
  return {static_cast<uint32_t>(val), 0, tag::character, 1};
}

template <>
inline gc::managed_ptr alloc<value::nil>()
{
  // 'nil' and nullptr are distinct values, since nil has the live flag set
  // (since some code, especially the C API, uses null checks to test for valid
  // values, and nil is of course valid).
  return {0, 0, tag::nil, 1};
}

template <>
inline gc::managed_ptr alloc<value::integer, int>(int&& val)
{
  return {static_cast<uint32_t>(val), 0, tag::integer, 1};
}

template <>
inline gc::managed_ptr alloc<value::integer, int&>(int& val)
{
  return {static_cast<uint32_t>(val), 0, tag::integer, 1};
}

template <>
inline gc::managed_ptr alloc<value::integer, const int&>(const int& val)
{
  return {static_cast<uint32_t>(val), 0, tag::integer, 1};
}

template <>
inline gc::managed_ptr alloc<value::integer>()
{
  return alloc<value::integer, int>(0);
}

// }}}

}

}

#endif
