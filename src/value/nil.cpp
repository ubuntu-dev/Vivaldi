#include "nil.h"

#include "gc.h"
#include "builtins.h"

#include <string>

using namespace vv;

value::nil::nil()
  : object {&builtin::type::nil}
{ }

std::string value::nil::value() const { return "nil"; }
