#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::builtin_function::builtin_function(
    const std::function<gc::managed_ptr(vm::machine&)>& body,
    size_t argc,
    bool takes_varargs)
  : basic_object {builtin::type::function},
    value        {body, argc, takes_varargs}
{ }
