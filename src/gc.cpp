#include "gc.h"

#include "gc/alloc.h"
#include "gc/block_list.h"
#include "gc/object_list.h"

#include "builtins.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/dictionary.h"
#include "value/function.h"
#include "value/object.h"
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

// Loaded dynamic libraries (i.e. C extensions); stored here to be destructed at
// the end of runtime. XXX: This _has_ to come before g_allocated, since the
// libraries have to be destructed last (so that no value::blob destructors try
// to call functions in dynamic libraries).
std::vector<dynamic_library> g_libs;

// Running VM (TODO: clearly this is pretty hacky. When multithreading is added,
// somewhere along the line, I'll need to support multiple VMs; until then, the
// GC interface is basically a subset of the VM interface and it's silly to keep
// them separate.
vm::machine* g_vm;

// Allocated blocks of memory.
gc::block_list g_blocks;
// List of allocated basic_objects.
gc::object_list g_allocated;

// Performs actual marking and sweeping, along with expanding available memory
// if we've genuinely run out.
void mark_sweep()
{
  const auto old_sz = g_allocated.size();

  g_vm->mark();

  const auto last = remove_if(std::begin(g_allocated), std::end(g_allocated),
                              [](auto* i)
  {
    if (g_blocks.is_marked(i))
      return false;
    g_blocks.reclaim(i, size_for(i->tag));
    destruct(*i);
    return true;
  });

  g_allocated.erase(last, std::end(g_allocated));
  g_blocks.unmark_all();

  // Expand memory if less than half was reclaimed (to avoid cases if, e.g.,
  // 50000 objects are marked and only 4 are swept, over and over again every
  // 4 allocations).
  if (old_sz - g_allocated.size() < g_allocated.size())
    g_blocks.expand();
}

}

// }}}
// External functions {{{

value::basic_object* gc::internal::get_next_empty(const tag type)
{
  auto ptr = g_blocks.allocate(size_for(type));
  if (!ptr) {
    mark_sweep();
    ptr = g_blocks.allocate(size_for(type));
  }

  const auto obj = reinterpret_cast<value::basic_object*>(ptr);
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
  for (auto i : env.members)
    gc::mark(*i.second);
}

void mark_object(value::object& obj)
{
  for (auto i : obj.members)
    gc::mark(*i.second);
}

}

void gc::mark(value::basic_object& obj)
{
  using namespace value;

  // Either already marked or stack-allocated
  if (!g_blocks.contains(&obj) || g_blocks.is_marked(&obj))
    return;
  g_blocks.mark(&obj);

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
  case tag::blob:
  case tag::object:          return mark_object(static_cast<object&>(obj));
  default:                   return;
  }
}

// }}}

