#include "array_iterator.h"

#include "builtins.h"
#include "gc.h"
#include "value/array.h"

using namespace vv;

value::array_iterator::array_iterator(gc::managed_ptr<array> new_arr)
  : object {&builtin::type::array_iterator},
    arr    {new_arr},
    idx    {0}
{ }

std::string value::array_iterator::value() const { return "<array iterator>"; }

void value::array_iterator::mark()
{
  object::mark();
  gc::mark(arr);
}
