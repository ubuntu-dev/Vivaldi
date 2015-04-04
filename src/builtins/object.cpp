#include "builtins/object.h"

#include "gc/alloc.h"
#include "utils/lang.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr object::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return gc::alloc<value::boolean>( vv::equals(self, arg) );
}

gc::managed_ptr object::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  return gc::alloc<value::boolean>( !equals(self, arg) );
}

gc::managed_ptr object::op_not(gc::managed_ptr self)
{
  return gc::alloc<value::boolean>( !truthy(self) );
}

gc::managed_ptr object::type(gc::managed_ptr self)
{
  return self.type();
}
