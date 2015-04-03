#include "array_iterator.h"

#include "builtins.h"

using namespace vv;

value::array_iterator::array_iterator(array& new_arr)
  : basic_object {&builtin::type::array_iterator, tag::array_iterator},
    arr          {new_arr},
    idx          {0}
{ }
