#include "range.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::range::range(basic_object* new_start, basic_object* new_end)
  : basic_object {&builtin::type::range, tag::range},
    start        {new_start},
    end          {new_end}
{ }

value::range::range()
  : basic_object {&builtin::type::range, tag::range},
    start        {nullptr},
    end          {nullptr}
{ }
