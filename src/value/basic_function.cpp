#include "value/basic_function.h"

#include "builtins.h"

using namespace vv;

value::basic_function::basic_function(vv::tag type,
                                      int argc,
                                      vm::environment* enclosing,
                                      vector_ref<vm::command> body)
  : basic_object {&builtin::type::function, type},
    argc         {argc},
    enclosing    {enclosing},
    body         {body}
{ }
