#include "nil.h"

#include "gc.h"
#include "builtins.h"

#include <string>

using namespace vv;

value::nil::nil()
  : object {&builtin::type::nil, tag::nil}
{ }
