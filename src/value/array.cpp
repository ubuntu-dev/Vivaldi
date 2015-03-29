#include "array.h"

#include "gc.h"
#include "builtins.h"

using namespace vv;

value::array::array(const std::vector<object*>& new_val)
  : object {&builtin::type::array},
    val {new_val}
{ }

std::string value::array::value() const
{
  std::string str{'['};
  if (val.size()) {
    for_each(begin(val), end(val) - 1,
             [&](const auto& v) { str += v->value() += ", "; });
    str += val.back()->value();
  }
  str += ']';
  return str;
}

void value::array::mark()
{
  object::mark();
  for (auto i : val)
    gc::mark(i);
}
