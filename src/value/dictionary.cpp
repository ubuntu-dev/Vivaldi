#include "dictionary.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::dictionary::dictionary(const std::unordered_map<base*, base*>& mems)
  : base {&builtin::type::dictionary},
    val  {mems}
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
  base::mark();
  for (auto& pair : val) {
    if (!pair.first->marked())
      gc::mark(*pair.first);
    if (!pair.second->marked())
      gc::mark(*pair.second);
  }
}
