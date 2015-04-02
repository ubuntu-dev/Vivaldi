#include "gc.h"

#include "gc/alloc.h"
#include "gc/allocated_block_list.h"
#include "gc/free_block_list.h"
#include "gc/object_list.h"

#include "builtins.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/dictionary.h"
#include "value/function.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/type.h"
#include "vm/call_frame.h"

#include <list>

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

std::list<std::array<char, 65'536>> g_blocks( 4 );

// Loaded dynamic libraries (i.e. C extensions); stored here to be destructed at
// the end of runtime. XXX: This _has_ to come before g_marked and g_unmarked,
// since the lirbraries have to be destructed first (so that no value::blob
// destructors try to call functions in dynamic libraries).
std::vector<dynamic_library> g_libs;

vm::machine* g_vm;

gc::allocated_block_list g_block_list;
gc::free_block_list g_free;
gc::object_list g_allocated;

void expand()
{
  for (auto i = g_blocks.size() / 2; i--;) {
    g_blocks.emplace_back();
    g_free.insert(g_blocks.back().data());
    g_block_list.insert_block(g_blocks.back().data());
  }
}

void copy_live()
{
  auto old_sz = g_allocated.size();

  g_vm->mark();

  auto start = partition(std::begin(g_allocated), std::end(g_allocated),
                         [](auto* i) { return g_block_list.marked(i); });
  for_each(start, std::end(g_allocated), [](auto* i)
  {
    g_free.reclaim(i, size_for(i->tag));
    destruct(*i);
  });
  g_allocated.erase(start, std::end(g_allocated));
  g_block_list.unmark();

  if (old_sz - g_allocated.size() < g_allocated.size() / 2)
    expand();
}

}

// }}}
// External functions {{{

value::object* gc::internal::get_next_empty(const tag type)
{
  auto ptr = g_free.allocate(size_for(type));
  if (!ptr) {
    copy_live();
    ptr = g_free.allocate(size_for(type));
  }

  const auto obj = reinterpret_cast<value::object*>(ptr);
  g_allocated.push_back(obj);
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
  for (auto& i : g_blocks) {
    g_free.insert(i.data());
    g_block_list.insert_block(i.data());
  }
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

  // Either already marked or stack-allocated
  if (!g_block_list.contains(&obj) || g_block_list.marked(&obj))
    return;
  g_block_list.mark(&obj);

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

