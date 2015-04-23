#include "partial_function.h"

#include "builtins.h"

using namespace vv;
using namespace value;

partial_function::partial_function(gc::managed_ptr func, gc::managed_ptr arg)
  : basic_object {builtin::type::function},
    value        {func, arg}
{ }
