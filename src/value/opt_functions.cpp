#include "opt_functions.h"

#include "builtins.h"

#include <array>

using namespace vv;

namespace {

const static std::array<vm::command, 1> g_builtin_shim {{
  { vm::instruction::ret, false }
}};

}

value::opt_monop::opt_monop(const std::function<base*(base*)>& body)
  : body {body}
{ }

std::string value::opt_monop::value() const { return "<builtin function>"; }

vector_ref<vm::command> value::opt_monop::get_body() const
{
  return g_builtin_shim;
}

value::opt_binop::opt_binop(const std::function<base*(base*, base*)>& body)
  : body {body}
{ }

std::string value::opt_binop::value() const { return "<builtin function>"; }

vector_ref<vm::command> value::opt_binop::get_body() const
{
  return g_builtin_shim;
}
