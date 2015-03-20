#include "function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::function::function(int argc,
                          const std::vector<vm::command>& new_body,
                          gc::managed_ptr<vm::environment> enclosing)
  : basic_function {func_type::vv, argc, enclosing, {}},
    vec_body       {new_body}
{
  body = vec_body;
}

std::string value::function::value() const { return "<function>"; }
