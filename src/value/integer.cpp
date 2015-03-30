#include "integer.h"

#include "gc.h"
#include "builtins.h"

#include <string>

using namespace vv;

value::integer::integer(int val)
  : object {&builtin::type::integer, tag::integer},
    val    {val}
{ }
