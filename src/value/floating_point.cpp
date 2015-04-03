#include "floating_point.h"

#include "builtins.h"

#include <string>

using namespace vv;

value::floating_point::floating_point(double value)
  : basic_object {&builtin::type::floating_point, tag::floating_point},
    val          {value}
{ }
