#include "value/regex.h"

#include "builtins.h"

using namespace vv;

value::regex::regex(const std::regex& val, const std::string& str)
  : basic_object {&builtin::type::regex, tag::regex},
    val          {val},
    str          {str}
{ }

value::regex_result::regex_result(string& str, std::smatch&& val)
  : basic_object     {&builtin::type::regex_result, tag::regex_result},
    val              {val},
    owning_str       {str}
{ }
