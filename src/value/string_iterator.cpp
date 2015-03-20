#include "string_iterator.h"

#include "builtins.h"
#include "gc.h"
#include "value/string.h"

using namespace vv;

value::string_iterator::string_iterator(gc::managed_ptr<string> new_str)
  : object {&builtin::type::string_iterator},
    str    {new_str},
    idx    {0}
{ }

std::string value::string_iterator::value() const
{
  return "<string iterator>";
}

void value::string_iterator::mark()
{
  object::mark();
  if (!str->marked())
    gc::mark(*str);
}
