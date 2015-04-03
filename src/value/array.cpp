#include "array.h"

#include "builtins.h"

using namespace vv;

value::array::array(const std::vector<basic_object*>& new_val)
  : basic_object {&builtin::type::array, tag::array},
    val          {new_val}
{ }
