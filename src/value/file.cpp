#include "value/file.h"

#include "builtins.h"

using namespace vv;

value::file::file(const std::string& filename)
  : basic_object {&builtin::type::file, tag::file},
    name         {filename},
    val          {filename}
{
  std::getline(val, cur_line);
}

value::file::file()
  : basic_object {&builtin::type::file, tag::file},
    name         {""}
{ }

value::file::file(file&& other)
  : basic_object {&builtin::type::file, tag::file},
    name         {move(other.name)},
    val          {std::move(other.val)}
{ }
