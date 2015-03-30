#include "value/string.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::string::string(const std::string& val)
  : object {&builtin::type::string, tag::string},
    val    {val}
{ }
