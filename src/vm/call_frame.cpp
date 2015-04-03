#include "call_frame.h"

#include "builtins.h"

using namespace vv;

vm::environment::environment(environment* new_enclosing, value::basic_object* new_self)
  : basic_object    {&builtin::type::object, tag::environment},
    enclosing       {new_enclosing},
    self            {new_self || !enclosing ? new_self : enclosing->self}
{ }

vm::call_frame::call_frame(vector_ref<vm::command> instr_ptr,
                           environment* env,
                           size_t argc,
                           size_t frame_ptr)
  : argc       {argc},
    frame_ptr  {frame_ptr},
    env        {env},
    caller     {nullptr},
    catcher    {nullptr},
    instr_ptr  {instr_ptr}
{ }
