#include "opt_functions.h"

#include "builtins.h"

#include <array>

using namespace vv;

namespace {

const static std::array<vm::command, 1> g_builtin_shim {{
  { vm::instruction::ret, false }
}};

}

value::opt_monop::opt_monop(const std::function<gc::managed_ptr(gc::managed_ptr)>& body)
  : basic_object {builtin::type::function},
    value        {body}
{ }

value::opt_binop::opt_binop(const std::function<gc::managed_ptr(gc::managed_ptr,
                                                                gc::managed_ptr)>& body)
  : basic_object {builtin::type::function},
    value        {body}
{ }
