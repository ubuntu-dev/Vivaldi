#include "value/regex.h"

#include "builtins.h"
#include "gc.h"
#include "value/string.h"

using namespace vv;

value::regex::regex(const std::regex& val, const std::string& str)
  : base {&builtin::type::regex},
    val  {val},
    str  {str}
{ }

std::string value::regex::value() const { return '`' + str + '`'; }

value::regex_result::regex_result(value::string& str, std::smatch&& val)
  : base       {&builtin::type::regex_result},
    val        {val},
    owning_str {str}
{ }

std::string value::regex_result::value() const { return "<regex result>"; }

void value::regex_result::mark()
{
  base::mark();
  gc::mark(owning_str);
}
