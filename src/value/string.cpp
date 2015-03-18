#include "value/string.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::string::string(const std::string& val)
  : object {&builtin::type::string},
    val    {val}
{ }

std::string value::string::value() const { return '"' + val += '"'; }

size_t value::string::hash() const
{
  const static std::hash<std::string> hasher{};
  return hasher(val);
}

bool value::string::equals(const object& other) const
{
  if (other.type != &builtin::type::string)
    return false;
  return static_cast<const string&>(other).val == val;
}
