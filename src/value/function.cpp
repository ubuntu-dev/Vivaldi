#include "function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::function::function(int argc,
                          const std::vector<vm::command>& new_body,
                          gc::managed_ptr enclosing)
  : basic_object {builtin::type::function},
    value        {new_body, argc, enclosing}
{ }
