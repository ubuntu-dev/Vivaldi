#include "array.h"

#include "gc.h"
#include "builtins.h"

using namespace vv;

value::array::array(const std::vector<object*>& new_val)
  : object {&builtin::type::array, tag::array},
    val {new_val}
{ }
