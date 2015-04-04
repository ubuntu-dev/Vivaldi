#include "value/file.h"

#include "builtins.h"

using namespace vv;

value::file::file(const std::string& filename)
  : basic_object {builtin::type::file},
    value        {filename, "", std::fstream{filename}}
{
  std::getline(value.val, value.cur_line);
}

value::file::file()
  : basic_object {builtin::type::file},
    value        {"", "", {}}
{ }

value::file::file(file&& other)
  : basic_object {builtin::type::file},
    value        ( std::move(other.value) )
{ }
