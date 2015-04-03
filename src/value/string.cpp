#include "value/string.h"

#include "builtins.h"

using namespace vv;

value::string::string(const std::string& val)
  : basic_object {&builtin::type::string, tag::string},
    val          {val}
{ }
