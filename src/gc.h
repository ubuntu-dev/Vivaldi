#ifndef VV_GC_H
#define VV_GC_H

#include "value.h"
#include "utils/dynamic_library.h"
#include "value/boolean.h"
#include "value/integer.h"
#include "value/nil.h"
#include "vm.h"

#include <array>

namespace vv {

namespace gc {

namespace internal {

value::base* get_next_empty();

template <typename T>
value::base* emplace(T&& item)
{
  auto slot = get_next_empty();
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
inline T* alloc(Args&&... args)
{
  static_assert(!std::is_same<T, value::boolean>::value, "unspecialized for bool");
  static_assert(!std::is_same<T, value::nil>::value, "unspecialized for nil");
  static_assert(!std::is_same<T, value::integer>::value, "unspecialized for integer");
  return static_cast<T*>(internal::emplace(T{std::forward<Args>(args)...}));
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

// }}}

void set_running_vm(vm::machine& vm);
vm::machine& get_running_vm();

dynamic_library& load_dynamic_library(const std::string& filename);

// Called in main at the start of the program
void init();

void mark(value::base& object);

}

}

#endif
