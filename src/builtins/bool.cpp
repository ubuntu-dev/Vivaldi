#include "builtins/boolean.h"

#include "gc/alloc.h"
#include "utils/lang.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr boolean::init(vm::machine& vm)
{
  vm.arg(0);
  if (vm.top().tag() == tag::boolean)
    return vm.top();
  return gc::alloc<value::boolean>( truthy(vm.top()) );
}
