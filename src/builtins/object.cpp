#include "builtins.h"

#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/boolean.h"
#include "value/opt_functions.h"
#include "value/object.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

namespace {

value::basic_object* fn_object_equals(value::basic_object* self, value::basic_object* arg)
{
  return gc::alloc<value::boolean>( equals(*self, *arg) );
}

value::basic_object* fn_object_unequal(value::basic_object* self, value::basic_object* arg)
{
  return gc::alloc<value::boolean>( !equals(*self, *arg) );
}

value::basic_object* fn_object_not(value::basic_object* self)
{
  return gc::alloc<value::boolean>( !truthy(*self) );
}

value::basic_object* fn_object_type(value::basic_object* self)
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
