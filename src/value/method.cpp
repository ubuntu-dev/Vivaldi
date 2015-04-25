#include "method.h"

#include "builtins.h"

using namespace vv;
using namespace value;

method::method(gc::managed_ptr func, gc::managed_ptr self)
  : basic_object {builtin::type::function},
    value        {func, self}
{ }
