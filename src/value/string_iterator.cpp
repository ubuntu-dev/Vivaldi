#include "string_iterator.h"

#include "builtins.h"

using namespace vv;

value::string_iterator::string_iterator(gc::managed_ptr str)
  : basic_object {builtin::type::string_iterator},
    value        {str, 0}
{ }
