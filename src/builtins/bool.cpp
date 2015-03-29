#include "builtins.h"

#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/boolean.h"
#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

value::object* fn_bool_init(vm::machine& vm)
{
  vm.arg(0);
  if (vm.top()->type == &type::boolean)
    return vm.top();
  return gc::alloc<value::boolean>( truthy(*vm.top()) );
}

value::builtin_function bool_init {fn_bool_init, 1};

}

value::type type::boolean {gc::alloc<value::boolean>, {
  { {"init"}, &bool_init }
}, builtin::type::object, {"Bool"}};
