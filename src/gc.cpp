#include "gc.h"

#include "gc/alloc.h"
#include "gc/block_list.h"
#include "gc/object_list.h"

#include "builtins.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/dictionary.h"
#include "value/function.h"
#include "value/method.h"
#include "value/object.h"
#include "value/partial_function.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/type.h"
#include "vm/call_frame.h"

using namespace vv;
using namespace gc;

// Internals {{{

// Allocated blocks of memory.
gc::block_list internal::g_blocks;

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

// List of allocated basic_objects.
gc::object_list g_allocated;

}

namespace {

// Performs actual marking and sweeping, along with expanding available memory
// if we've genuinely run out.
void mark_sweep()
{
  const auto old_sz = g_allocated.size();

  g_vm->mark();

  const auto last = remove_if(std::begin(g_allocated), std::end(g_allocated),
                              [](auto i)
  {
    if (internal::g_blocks.is_marked(i))
      return false;
    internal::g_blocks.reclaim(i, size_for(i.tag()));
    clear_members(i);
    destroy(i);
    return true;
  });

  g_allocated.erase(last, std::end(g_allocated));
  internal::g_blocks.unmark_all();

  // Expand memory if less than half was reclaimed (to avoid cases if, e.g.,
  // 50000 objects are marked and only 4 are swept, over and over again every
  // 4 allocations).
  if (old_sz - g_allocated.size() < g_allocated.size())
    internal::g_blocks.expand();
}

}

// }}}
// External functions {{{

gc::managed_ptr gc::internal::get_next_empty(const tag type)
{
  auto ptr = g_blocks.allocate(size_for(type));
  if (!ptr) {
    mark_sweep();
    ptr = g_blocks.allocate(size_for(type));
  }

  ptr.m_tag = type;
  g_allocated.push_back(ptr);
  return ptr;
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

namespace {

void mark_array(gc::managed_ptr array)
{
  for (auto i : value::get<value::array>(array))
    mark(i);
}

void mark_dictionary(gc::managed_ptr dictionary)
{
  for (auto i : value::get<value::dictionary>(dictionary)) {
    mark(i.first);
    mark(i.second);
  }
}

void mark_method(gc::managed_ptr method)
{
  mark(value::get<value::method>(method).function);
  mark(value::get<value::method>(method).self);
}

void mark_partial_function(gc::managed_ptr function)
{
  mark(value::get<value::partial_function>(function).function);
  mark(value::get<value::partial_function>(function).provided_arg);
}

void mark_range(gc::managed_ptr rng)
{
  mark(value::get<value::range>(rng).start);
  mark(value::get<value::range>(rng).end);
}

void mark_type(gc::managed_ptr type)
{
  mark(value::get<value::type>(type).parent);
  for (auto i : value::get<value::type>(type).methods)
    mark(i.second);
}

void mark_environment(gc::managed_ptr env)
{
  mark(value::get<vm::environment>(env).enclosing);
  mark(value::get<vm::environment>(env).self);
  for (auto i : value::get<vm::environment>(env).members)
    gc::mark(i.second);
}

}

void gc::mark(managed_ptr obj)
{
  using namespace value;

  if (!obj)
    return;

  if (obj.tag() == tag::boolean || obj.tag() == tag::character ||
      obj.tag() == tag::integer || obj.tag() == tag::nil) {
    mark(obj.type());
    mark_members(obj);
    return;
  }

  // Either already marked or stack-allocated
  if (internal::g_blocks.is_marked(obj))
    return;

  internal::g_blocks.mark(obj);

  mark(obj.type());
  mark_members(obj);

  switch (obj.tag()) {
  case tag::array:            return mark_array(obj);
  case tag::array_iterator:   return mark(get<array_iterator>(obj).arr);
  case tag::dictionary:       return mark_dictionary(obj);
  case tag::function:         return mark(get<function>(obj).enclosure);
  case tag::method:           return mark_method(obj);
  case tag::partial_function: return mark_partial_function(obj);
  case tag::range:            return mark_range(obj);
  case tag::regex_result:     return mark(get<regex_result>(obj).owning_str);
  case tag::string_iterator:  return mark(get<string_iterator>(obj).str);
  case tag::type:             return mark_type(obj);
  case tag::environment:      return mark_environment(obj);
  default:                    return;
  }
}

// }}}

