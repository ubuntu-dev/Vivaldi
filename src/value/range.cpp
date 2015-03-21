#include "range.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::range::range(object_ptr new_start, object_ptr new_end)
  : object {&builtin::type::range},
    start  {new_start},
    end    {new_end}
{ }

value::range::range()
  : object {&builtin::type::range},
    start  {nullptr},
    end    {nullptr}
{ }

std::string value::range::value() const
{
  return start->value() + " to " + end->value();
}

void value::range::mark()
{
  object::mark();
  // We need to ensure neither start not end are nullptr, since this could be
  // happening between allocation and initialization
  if (start)
    gc::mark(start);
  if (end)
    gc::mark(end);
}
