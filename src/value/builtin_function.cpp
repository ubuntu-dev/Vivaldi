#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

namespace {

const static std::array<vm::command, 1> builtin_shim {{
  { vm::instruction::ret, false }
}};

}

value::builtin_function::builtin_function(
    const std::function<gc::managed_ptr(vm::machine&)>& body,
    size_t argc)
  : basic_object {builtin::type::function},
    value        {body, argc}
{ }
