#include "dictionary.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::dictionary::dictionary(const std::unordered_map<object_ptr, object_ptr>& mems)
  : object {&builtin::type::dictionary},
    val    {mems}
{ }

std::string value::dictionary::value() const
{
  std::string str{"{"};
  for (const auto& pair: val)
    str += ' ' + pair.first->value() += ": " + pair.second->value() += ',';
  if (val.size())
    str.back() = ' ';
  return str += '}';
}

void value::dictionary::mark()
{
  object::mark();
  for (auto& pair : val) {
    gc::mark(pair.first);
    gc::mark(pair.second);
  }
}
