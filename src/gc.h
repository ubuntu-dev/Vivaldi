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

value::base* emplace(value::base* item);

// Optimize common values
extern value::nil g_nil;
extern value::boolean g_true;
extern value::boolean g_false;
extern std::array<value::integer, 1024> g_ints;

}

template <typename T, typename... Args>
inline T* alloc(Args&&... args)
{
  return static_cast<T*>(internal::emplace(new T{args...}));
}

// Optimized template overrides for alloc:
template <>
inline value::boolean* alloc<value::boolean>(bool&& val)
{
  return &(val ? internal::g_true : internal::g_false);
}

template <>
inline value::nil* alloc<value::nil>()
{
  return &internal::g_nil;
}

template <>
inline value::integer* alloc<value::integer>(int&& val)
{
  if (val >= 0 && val < 1024)
    return &internal::g_ints[static_cast<unsigned>(val)];
  return gc::alloc<value::integer>( val );
}

void set_running_vm(vm::machine& vm);

// Called in main at the start and end of the program. TODO: RAII
void init();
void empty();

}

}

#endif
