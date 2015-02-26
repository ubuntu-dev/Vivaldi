#include "value/file.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::file::file(const std::string& filename)
  : base {&builtin::type::file},
    name {filename},
    val  {new std::fstream{filename}}
{
  std::getline(*val, cur_line);
}

value::file::file()
  : base {&builtin::type::file},
    name {""}
{ }

value::file::file(file&& other)
  : base {&builtin::type::file},
    name {move(other.name)},
    val  {other.val}
{
  other.val = nullptr;
}

std::string value::file::value() const { return "File: " + name; }

value::file::~file()
{
  delete val;
}
