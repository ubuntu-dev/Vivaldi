#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
#include "value/opt_functions.h"

using namespace vv;
using namespace builtin;

namespace {

value::base* fn_object_equals(value::base* self, value::base* arg)
{
  return gc::alloc<value::boolean>( self->equals(*arg) );
}

value::base* fn_object_unequal(value::base* self, value::base* arg)
{
  return gc::alloc<value::boolean>( !self->equals(*arg) );
}

value::base* fn_object_not(value::base* self)
{
  return gc::alloc<value::boolean>( !truthy(self) );
}

value::base* fn_object_type(value::base* self)
{
  return self->type;
}

value::opt_binop obj_equals  {fn_object_equals };
value::opt_binop obj_unequal {fn_object_unequal};
value::opt_monop obj_not     {fn_object_not    };
value::opt_monop obj_type    {fn_object_type   };
}
value::type type::object {gc::alloc<value::base>, {
  { {"equals"},  &obj_equals },
  { {"unequal"}, &obj_unequal },
  { {"not"},     &obj_not },
  { {"type"},    &obj_type }
}, builtin::type::object, {"Object"}};

