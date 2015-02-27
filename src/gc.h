#ifndef VV_GC_H
#define VV_GC_H

#include "value.h"
#include "value/boolean.h"
#include "value/integer.h"
#include "value/nil.h"
#include "vm.h"

#include <array>

namespace vv {

namespace gc {

namespace internal {

union value_type;

void set_value_type(value_type* val, value::array&& array);
void set_value_type(value_type* val, value::array_iterator&& array_iterator);
void set_value_type(value_type* val, value::base&& base);
void set_value_type(value_type* val, value::builtin_function&& builtin_function);
void set_value_type(value_type* val, value::dictionary&& dictionary);
void set_value_type(value_type* val, value::file&& file);
void set_value_type(value_type* val, value::floating_point&& floating_point);
void set_value_type(value_type* val, value::function&& function);
void set_value_type(value_type* val, value::integer&& integer);
void set_value_type(value_type* val, value::range&& range);
void set_value_type(value_type* val, value::string&& string);
void set_value_type(value_type* val, value::string_iterator&&  string_iterator);
void set_value_type(value_type* val, value::symbol&& symbol);
void set_value_type(value_type* val, value::type&& type);
void set_value_type(value_type* val, vm::environment&& environment);

// Optimize common values
extern value::nil g_nil;
extern value::boolean g_true;
extern value::boolean g_false;
extern std::array<value::integer, 1024> g_ints;

void mark();
void sweep();

value_type* get_next_empty();

template <typename T>
inline value::base* emplace(T&& val)
{
  auto empty = get_next_empty();
  set_value_type(empty, std::move(val));
  return reinterpret_cast<value::base*>(empty);
}

}

template <typename T, typename... Args>
inline T* alloc(Args&&... args)
{
  static_assert(!std::is_same<T, value::boolean>::value, "unspecialized for bool");
  static_assert(!std::is_same<T, value::nil>::value, "unspecialized for nil");
  static_assert(!std::is_same<T, value::integer>::value, "unspecialized for integer");
  return static_cast<T*>(internal::emplace(T{args...}));
}

// Optimized template overrides for alloc:
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
  return static_cast<value::integer*>(internal::emplace(value::integer{val}));
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

void set_running_vm(vm::machine& vm);

// Called in main at the start and end of the program. TODO: RAII
void init();
void empty();

}

}

#endif
