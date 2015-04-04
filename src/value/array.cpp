#include "array.h"

#include "builtins.h"

using namespace vv;

value::array::array(const std::vector<gc::managed_ptr>& val)
  : basic_object {builtin::type::array},
    value        {val}
{ }
