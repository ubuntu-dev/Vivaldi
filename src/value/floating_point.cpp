#include "floating_point.h"

#include "builtins.h"
#include "gc.h"

#include <string>

using namespace vv;

value::floating_point::floating_point(double value)
  : object {&builtin::type::floating_point, tag::floating_point},
    val    {value}
{ }
