#include "opt_functions.h"

#include "builtins.h"

#include <array>

using namespace vv;

namespace {

const static std::array<vm::command, 1> g_builtin_shim {{
  { vm::instruction::ret, false }
}};

}

value::opt_monop::opt_monop(const std::function<object*(object*)>& body)
  : basic_function {tag::opt_monop, 0, nullptr, g_builtin_shim},
    fn_body        {body}
{ }

value::opt_binop::opt_binop(const std::function<object*(object*, object*)>& body)
  : basic_function {tag::opt_binop, 1, nullptr, g_builtin_shim},
    fn_body        {body}
{ }
