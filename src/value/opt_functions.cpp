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
  : basic_function {func_type::opt1, 0, nullptr, g_builtin_shim},
    fn_body        {body}
{ }

std::string value::opt_monop::value() const { return "<builtin function>"; }

value::opt_binop::opt_binop(const std::function<object*(object*, object*)>& body)
  : basic_function {func_type::opt2, 1, nullptr, g_builtin_shim},
    fn_body        {body}
{ }

std::string value::opt_binop::value() const { return "<builtin function>"; }
