#include "gc.h"

#include "gc/alloc.h"
#include "gc/managed_ptr.h"

#include "builtins.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/blob.h"
#include "value/builtin_function.h"
#include "value/boolean.h"
#include "value/dictionary.h"
#include "value/integer.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/nil.h"
#include "value/object.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"
#include "value/type.h"
#include "vm/call_frame.h"

#include <list>
#include <iostream>

using namespace vv;
using namespace gc;
using namespace internal;

// Constants {{{

value::nil gc::internal::g_nil{};
value::boolean gc::internal::g_true{true};
value::boolean gc::internal::g_false{false};
std::array<value::integer, 1024> gc::internal::g_ints;

// }}}

// Internals {{{

namespace {

// Loaded dynamic libraries (i.e. C extensions); stored here to be destructed at
// the end of runtime. XXX: This _has_ to come before g_vals, since g_vals needs
// to be destructed first (so that no value::blob destructors try to call
// functions in dynamic libraries).
std::vector<dynamic_library> g_libs;

vm::machine* g_vm;

std::list<std::array<char, 65'536>> g_blocks( 4 );

vv::free_block_list g_free;
gc::allocated_block_list g_marked;
gc::allocated_block_list g_unmarked;

void copy_live()
{
  g_vm->mark();
  std::swap(g_marked, g_unmarked);
  while (g_marked.size()) {
    g_free.insert(g_marked.erase_destruct(std::begin(g_marked)));
  }
}

void expand()
{
  for (auto i = g_blocks.size(); i--;) {
    g_blocks.emplace_back();
    g_free.insert({g_blocks.back().data(), g_blocks.back().size()});
  }
}

}

// }}}
// External functions {{{

value::object* gc::internal::get_next_empty(tag type)
{
  static auto iter = std::begin(g_free);

  auto sz = size_for(type); // Why the *** ****** *** **** is this not inlined?
  auto search = [sz](auto block) { return block.second >= sz; };

  iter = find_if(iter, std::end(g_free), search);
  if (iter == std::end(g_free)) {
    iter = find_if(std::begin(g_free), iter, search);
    if (iter == std::end(g_free)) {
      copy_live();
      iter = find_if(std::begin(g_free), std::end(g_free), search);

      if (iter == std::end(g_free)) {
        expand();
        iter = find_if(std::begin(g_free), std::end(g_free), search);
      }
    }
  }

  auto ptr = iter->first;

  // add remaining space to free list
  {
    auto block_sz = iter->second;
    iter = g_free.erase(iter);
    if (block_sz > sz)
      iter = g_free.insert({ptr + sz, block_sz - sz});
  }

  auto obj = reinterpret_cast<value::object*>(ptr);
  g_unmarked.insert(obj);
  return obj;
}

void gc::set_running_vm(vm::machine& vm)
{
  g_vm = &vm;
}

vm::machine& gc::get_running_vm()
{
  return *g_vm;
}

dynamic_library& gc::load_dynamic_library(const std::string& filename)
{
  g_libs.emplace_back(filename);
  return g_libs.back();
}

void gc::init()
{
  int value = 0;
  for (auto& i : internal::g_ints)
    i.val = value++;

  for (auto& i : g_blocks)
    g_free.insert({i.data(), i.size()});
}

namespace {

void mark_array(value::array& array)
{
  for (auto i : array.val)
    mark(*i);
}

void mark_dictionary(value::dictionary& dictionary)
{
  for (auto i : dictionary.val) {
    mark(*i.first);
    mark(*i.second);
  }
}

void mark_range(value::range& rng)
{
  if (rng.start)
    mark(*rng.start);
  if (rng.end)
    mark(*rng.end);
}

void mark_type(value::type& type)
{
  mark(type.parent);
  for (auto i : type.methods)
    mark(*i.second);
}

void mark_environment(vm::environment& env)
{
  if (env.enclosing)
    mark(*env.enclosing);
  if (env.self)
    mark(*env.self);
}

}

void gc::mark(value::object& obj)
{
  using namespace value;

  if (!g_unmarked.count(&obj))
    return;

  auto val = g_unmarked.erase(&obj);
  g_marked.insert(val);

  for (auto i : obj.members)
    mark(*i.second);

  mark(*obj.type);

  switch (obj.tag) {
  case tag::array:           return mark_array(static_cast<array&>(obj));
  case tag::array_iterator:  return mark(static_cast<array_iterator&>(obj).arr);
  case tag::dictionary:      return mark_dictionary(static_cast<dictionary&>(obj));
  case tag::function:        return mark(*static_cast<function&>(obj).enclosing);
  case tag::range:           return mark_range(static_cast<range&>(obj));
  case tag::regex_result:    return mark(static_cast<regex_result&>(obj).owning_str);
  case tag::string_iterator: return mark(static_cast<string_iterator&>(obj).str);
  case tag::type:            return mark_type(static_cast<type&>(obj));
  case tag::environment:     return mark_environment(static_cast<vm::environment&>(obj));
  default:                   return;
  }
}

// }}}
