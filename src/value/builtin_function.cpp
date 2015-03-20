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
    const std::function<object_ptr(vm::machine&)>& body,
    int argc)
  : basic_function {func_type::builtin, argc, nullptr, builtin_shim},
    fn_body        {body}
{ }

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}
