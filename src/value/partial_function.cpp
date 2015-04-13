#include "partial_function.h"

#include "builtins.h"

using namespace vv;
using namespace value;

partial_function::partial_function(gc::managed_ptr function,
                                   const std::vector<gc::managed_ptr>& args)
  : basic_object {builtin::type::function},
    value        {function, args}
{ }
