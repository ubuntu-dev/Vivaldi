#ifndef VV_GC_ALLOC_H
#define VV_GC_ALLOC_H

#include "symbol.h"
#include "value.h"
#include "gc/managed_ptr.h"

namespace vv {

namespace gc {

namespace internal {

managed_ptr get_next_empty(tag type, size_t sz);

}

template <typename T, typename... Args>
gc::managed_ptr alloc(Args&&... args)
{
  static_assert(!std::is_same<T, value::boolean>(),   "unspecialized for bool");
  static_assert(!std::is_same<T, value::character>(), "unspecialized for char");
  static_assert(!std::is_same<T, value::nil>(),       "unspecialized for nil");
  static_assert(!std::is_same<T, value::integer>(),   "unspecialized for integer");
  static_assert(!std::is_same<T, value::symbol>(),    "unspecialized for symbol");

  auto slot = internal::get_next_empty(tag_for<T>(), sizeof(T));
  new (slot.get()) T{std::forward<Args>(args)...};
  return slot;
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
inline gc::managed_ptr alloc<value::integer, value::integer>(value::integer&& val)
{
  return {static_cast<uint32_t>(val >> 16),
          static_cast<uint16_t>(val & 0xffff),
          tag::integer, 1};
}

template <>
inline gc::managed_ptr alloc<value::integer, value::integer&>(value::integer& val)
{
  return alloc<value::integer, value::integer>(std::move(val));
}

template <>
inline gc::managed_ptr alloc<value::integer, const value::integer&>(const value::integer& val)
{
  return alloc<value::integer, value::integer>(value::integer{val});
}

template <>
inline gc::managed_ptr alloc<value::integer>()
{
  return alloc<value::integer, value::integer>(0);
}

template <>
inline gc::managed_ptr alloc<value::symbol, std::string_view>(std::string_view&& val)
{
  auto sym = symbol::s_symbol_table.find(val);
  if (sym != std::end(symbol::s_symbol_table))
    return sym->second;

  auto slot = internal::get_next_empty(tag::symbol, sizeof(size_t) + val.size() + 1);
  const auto size_ptr = reinterpret_cast<size_t*>(slot.get());
  const auto char_ptr = reinterpret_cast<char*>(slot.get()) + sizeof(size_t);
  *size_ptr = val.size();
  std::copy(begin(val), end(val), char_ptr);
  char_ptr[val.size()] = '\0';
  symbol::s_symbol_table.emplace(to_string(symbol{slot}), slot);
  return slot;
}

template <>
inline gc::managed_ptr alloc<value::symbol, std::string_view&>(std::string_view& val)
{
  return alloc<value::symbol, std::string_view>(std::move(val));
}

template <>
inline gc::managed_ptr alloc<value::symbol, const std::string_view&>(const std::string_view& val)
{
  return alloc<value::symbol, std::string_view>(std::string_view{val});
}

template <>
inline gc::managed_ptr alloc<value::symbol>()
{
  return alloc<value::symbol, std::string_view>({});
}

// }}}

}

}

#endif
