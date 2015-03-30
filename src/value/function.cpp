#include "function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::function::function(int argc,
                          const std::vector<vm::command>& new_body,
                          vm::environment* enclosing)
  : basic_function {tag::function, argc, enclosing, {}},
    vec_body       {new_body}
{
  body = vec_body;
}
