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
    const std::function<object*(vm::machine&)>& body,
    int argc)
  : basic_function {tag::builtin_function, argc, nullptr, builtin_shim},
    fn_body        {body}
{ }
