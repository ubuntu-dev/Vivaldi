#include "function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

// in principle the takes_varargs arg is redundant, since we could examine the
// body to figure out if it contains a varg instruction, but that could be
// dangerous in case the varg is optimized out--- we still want to accept
// functions with more than `argc` arguments!--- so having it be an explicit
// parameter is more future-proof.
value::function::function(int argc,
                          const std::vector<vm::command>& new_body,
                          gc::managed_ptr enclosing,
                          bool takes_varargs)
  : basic_object  {builtin::type::function},
    value         {new_body, argc, enclosing, takes_varargs}
{ }
