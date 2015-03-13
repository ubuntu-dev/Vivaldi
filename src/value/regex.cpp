#include "value/regex.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::regex::regex(const std::regex& val)
  : base {&builtin::type::regex},
    val  {val}
{ }

std::string value::regex::value() const { return "<regex>"; }
