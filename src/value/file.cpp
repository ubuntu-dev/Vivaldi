#include "value/file.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::file::file(const std::string& filename)
  : object {&builtin::type::file},
    name   {filename},
    val    {filename}
{
  std::getline(val, cur_line);
}

value::file::file()
  : object {&builtin::type::file},
    name   {""}
{ }

value::file::file(file&& other)
  : object {&builtin::type::file},
    name   {move(other.name)},
    val    {std::move(other.val)}
{ }

std::string value::file::value() const { return "File: " + name; }
