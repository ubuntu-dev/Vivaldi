#include "function.h"

#include "builtins.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/function.h"
#include "value/partial_function.h"

using namespace vv;
using namespace builtin;

namespace {

size_t get_argc(gc::managed_ptr fn)
{
  switch (fn.tag()) {
  case tag::opt_monop:        return 0;
  case tag::opt_binop:        return 1;
  case tag::builtin_function: return value::get<value::builtin_function>(fn).argc;
  case tag::function:         return value::get<value::function>(fn).argc;
  default: return get_argc(value::get<value::partial_function>(fn).function) + 1;
  }
}

bool takes_varargs(gc::managed_ptr fn)
{
  switch (fn.tag()) {
  case tag::builtin_function: return value::get<value::builtin_function>(fn).takes_varargs;
  case tag::function:         return value::get<value::function>(fn).takes_varargs;
  default:                    return false;
  }
}

}

gc::managed_ptr function::bind(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (get_argc(self) == 0 && !takes_varargs(self)) {
    return throw_exception(type::range_error,
                           "Function takes no arguments to bind to");
  }

  return gc::alloc<value::partial_function>( self, arg );
}

// TODO: support non-Array arg lists
gc::managed_ptr function::apply(vm::machine& vm)
{
  vm.self();
  const auto& self = vm.top();
  vm.arg(0);
  const auto& arg = vm.top();
  vm.pop(2);
  if (arg.type() != builtin::type::array)
    return throw_exception(type::type_error, "Argument list must be an Array");

  for_each(rbegin(value::get<value::array>(arg)), rend(value::get<value::array>(arg)),
           [&vm](const auto& i) { vm.push(i); });

  vm.push(self);
  vm.call(value::get<value::array>(arg).size());
  vm.run_cur_scope();
  return vm.top();
}
