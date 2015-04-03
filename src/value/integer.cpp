#include "integer.h"

#include "builtins.h"

#include <string>

using namespace vv;

value::integer::integer(int val)
  : basic_object {&builtin::type::integer, tag::integer},
    val          {val}
{ }
