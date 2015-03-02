#include "builtin_function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::builtin_function::builtin_function(
    const std::function<base*(vm::machine&)>& new_body,
    int new_argc)
  : body {new_body},
    argc {new_argc}
{ }

std::string value::builtin_function::value() const
{
  return "<builtin function>";
}

int value::builtin_function::get_argc() const { return argc; }

vector_ref<vm::command> value::builtin_function::get_body() const
{
  const static std::array<vm::command, 1> builtin_shim {{
    { vm::instruction::ret, false }
  }};
  return builtin_shim;
}

vm::environment* value::builtin_function::get_enclosing() const
{
  return nullptr;
}
