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
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"

#include <cassert>
#include <list>
#include <stack>

using namespace vv;
using namespace gc;
using namespace internal;

// Constants {{{

value::nil gc::internal::g_nil{};
value::boolean gc::internal::g_true{true};
value::boolean gc::internal::g_false{false};
std::array<value::integer, 1024> gc::internal::g_ints;

// }}}

// value_type {{{

namespace {

// Think of this as just a chunk of memory with ~256 bytes; it's done as a union
// to ensure that we get the proper maximum size and don't inadvertently slice
// anything.  Every builtin type (except nil and bool, since they're handled
// separately) should be included.
union value_type {
  value::array            array;
  value::array_iterator   array_iterator;
  value::object           object;
  value::blob             blob;
  value::builtin_function builtin_function;
  value::dictionary       dictionary;
  value::file             file;
  value::floating_point   floating_point;
  value::function         function;
  value::integer          integer;
  value::opt_monop        monop;
  value::opt_binop        binop;
  value::range            range;
  value::string           string;
  value::string_iterator  string_iterator;
  value::symbol           symbol;
  value::type             type;
  vm::environment         environment;
  // Used by default ctor, and for empty slots. We know that value::object and its
  // derived classes can never be all zeroes, since it includes several
  // nonnullable pointers/references, so testing for zero works as a means of
  // testing emptiness.
  size_t blank;

  value_type() : blank{} { }

  bool marked() const { return object.marked(); }
  void unmark() { object.unmark(); }

  bool empty() const { return blank == 0; }
  void clear()
  {
    if (!empty()) {
      (&object)->~object();
      blank = 0;
    }
  }

  // In a sense this union is tagged, by virtue of vtables
  ~value_type() { if (!empty()) (&object)->~object(); }
};

}

// }}}
// block {{{

struct internal::block {
  std::bitset<512> mark_bits;
  std::array<value_type, 512> values;
};

// }}}
// Internals {{{

namespace {

// Loaded dynamic libraries (i.e. C extensions); stored here to be destructed at
// the end of runtime. XXX: This _has_ to come before g_vals, since g_vals needs
// to be destructed first (so that no value::blob destructors try to call
// functions in dynamic libraries).
std::vector<dynamic_library> g_libs;

std::stack<managed_ptr<value_type>> g_free;

// Using list, instead of vector, for two reasons:
// - copying value_type's is both infeasible and expensive
// - pointers to value_type's (i.e. iterators in g_vals) can never be
//   invalidated, lest everything blow up
std::list<internal::block> g_vals( 4 );

// Stored here for GC marking
vm::machine* g_vm;

void mark()
{
  if (g_vm)
    g_vm->mark();
}

void copy_free()
{
  for (auto& block : g_vals) {
    for (auto& i : block.values) {
      if (i.marked())
        i.unmark();
      else
        g_free.emplace(&i, block);
    }
  }
}

}

// }}}
// External functions {{{

managed_ptr<value::object> internal::get_next_empty()
{
  if (!g_free.size()) {
    ::mark();
    copy_free();
  }

  if (!g_free.size()) {
    auto i = --end(g_vals);
    g_vals.resize(g_vals.size() * 2);
    for_each(++i, end(g_vals),
             [&](auto& block) { for (auto& i : block.values) g_free.push(&i); });
  }

  auto val = g_free.top();
  g_free.pop();
  val->clear();
  return {&val->object, val.block()};
}

void gc::set_running_vm(vm::machine& vm)
{
  g_vm = &vm;
}

vm::machine& gc::get_running_vm()
{
  assert(g_vm);
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
  for (auto& block : g_vals)
    for (auto& i : block.values)
      g_free.push(&i);
}

void gc::mark(value::object& object)
{
  object.mark();
}

// }}}
