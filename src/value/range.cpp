#include "range.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::range::range(gc::managed_ptr start, gc::managed_ptr end)
  : basic_object {builtin::type::range},
    value        {start, end}
{ }

value::range::range()
  : basic_object {builtin::type::range},
    value        {}
{ }
