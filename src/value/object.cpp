#include "object.h"

#include "builtins.h"

using namespace vv;

value::object::object()
  : basic_object {&builtin::type::object, tag::object},
    members      {}
{ }
