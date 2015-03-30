#include "object.h"

#include "builtins.h"

using namespace vv;

value::object::object(value::type* new_type, vv::tag tag)
  : tag     {tag},
    members {},
    type    {new_type}
{ }

value::object::object()
  : tag     {tag::object},
    members {},
    type    {&builtin::type::object}
{ }
