#include "boolean.h"

#include "builtins.h"
#include "gc.h"

#include <string>

using namespace vv;

value::boolean::boolean(bool new_val)
  : object {&builtin::type::boolean, tag::boolean},
    val    {new_val}
{ }
