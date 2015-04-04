#include "array_iterator.h"

#include "builtins.h"

using namespace vv;

value::array_iterator::array_iterator(gc::managed_ptr arr)
  : basic_object {builtin::type::array_iterator},
    value        {arr, 0}
{ }
