#include "value/regex.h"

#include "builtins.h"

using namespace vv;

value::regex::regex(const std::regex& val, const std::string& str)
  : basic_object {builtin::type::regex},
    value        {val, str}
{ }

value::regex_result::regex_result(gc::managed_ptr str, std::smatch&& val)
  : basic_object     {builtin::type::regex_result},
    value            {val, str}
{ }
