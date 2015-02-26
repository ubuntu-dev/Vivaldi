#ifndef VV_GC_H
#define VV_GC_H

#include "value.h"
#include "value/boolean.h"
#include "value/integer.h"
#include "value/nil.h"
#include "vm.h"

#include "builtins.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/builtin_function.h"
#include "value/boolean.h"
#include "value/dictionary.h"
#include "value/integer.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/nil.h"
#include "value/range.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"

#include <array>
#include <list>

namespace vv {

namespace gc {

namespace internal {

union value_type {
  value::array            array;
  value::array_iterator   array_iterator;
  value::base             base;
  value::boolean          boolean;
  value::builtin_function builtin_function;
  value::dictionary       dictionary;
  value::file             file;
  value::floating_point   floating_point;
  value::function         function;
  value::integer          integer;
  value::nil              nil;
  value::range            range;
  value::string           string;
  value::string_iterator  string_iterator;
  value::symbol           symbol;
  value::type             type;

  value_type& operator=(value::array&& array);
  value_type& operator=(value::array_iterator&& array_iterator);
  value_type& operator=(value::base&& base);
  value_type& operator=(value::boolean&& boolean);
  value_type& operator=(value::builtin_function&& builtin_function);
  value_type& operator=(value::dictionary&& dictionary);
  value_type& operator=(value::file&& file);
  value_type& operator=(value::floating_point&& floating_point);
  value_type& operator=(value::function&& function);
  value_type& operator=(value::integer&& integer);
  value_type& operator=(value::nil&& nil);
  value_type& operator=(value::range&& range);
  value_type& operator=(value::string&& string);
  value_type& operator=(value::string_iterator&&  string_iterator);
  value_type& operator=(value::symbol&& symbol);
  value_type& operator=(value::type&& type);
  value_type& operator=(vm::environment&& environment);

  value_type() : nil{} { }

  bool marked() const { return base.marked(); }
  void unmark() { base.unmark(); }
  void mark() { (&base)->mark(); }

  bool empty() const { return base.type == &builtin::type::nil; }

  operator value::base& () { return base; }

  ~value_type() { (&base)->~base(); }
};

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
  //auto first = try_emplace(val);
  auto empty = get_next_empty();
  *empty = std::move(val);
  return &empty->base;
}

}

template <typename T, typename... Args>
inline T* alloc(Args&&... args)
{
  //return static_cast<T*>(internal::emplace(new T{args...}));
  return static_cast<T*>(internal::emplace(T{args...}));
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
