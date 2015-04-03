#include "nil.h"

#include "builtins.h"

using namespace vv;

value::nil::nil()
  : basic_object {&builtin::type::nil, tag::nil}
{ }
