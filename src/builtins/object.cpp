#include "builtins.h"

#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/boolean.h"
#include "value/opt_functions.h"

using namespace vv;
using namespace builtin;

namespace {

value::object* fn_object_equals(value::object* self, value::object* arg)
{
  return gc::alloc<value::boolean>( self->equals(*arg) );
}

value::object* fn_object_unequal(value::object* self, value::object* arg)
{
  return gc::alloc<value::boolean>( !self->equals(*arg) );
}

value::object* fn_object_not(value::object* self)
{
  return gc::alloc<value::boolean>( !truthy(*self) );
}

value::object* fn_object_type(value::object* self)
{
  return self->type;
}

value::opt_binop obj_equals  {fn_object_equals };
value::opt_binop obj_unequal {fn_object_unequal};
value::opt_monop obj_not     {fn_object_not    };
value::opt_monop obj_type    {fn_object_type   };
}
value::type type::object {gc::alloc<value::object>, {
  { {"equals"},  &obj_equals },
  { {"unequal"}, &obj_unequal },
  { {"not"},     &obj_not },
  { {"type"},    &obj_type }
}, builtin::type::object, {"Object"}};
