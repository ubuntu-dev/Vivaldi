#include "range.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::range::range(object* new_start, object* new_end)
  : object {&builtin::type::range, tag::range},
    start  {new_start},
    end    {new_end}
{ }

value::range::range()
  : object {&builtin::type::range, tag::range},
    start  {nullptr},
    end    {nullptr}
{ }
