#include "builtins/boolean.h"

#include "gc/alloc.h"
#include "utils/lang.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr boolean::init(gc::managed_ptr, gc::managed_ptr arg)
{
  if (arg.tag() == tag::boolean)
    return arg;
  return gc::alloc<value::boolean>( truthy(arg) );
}
