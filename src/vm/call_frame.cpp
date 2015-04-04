#include "call_frame.h"

#include "builtins.h"

using namespace vv;

vm::environment::environment(gc::managed_ptr enclosing, gc::managed_ptr self)
  : basic_object {builtin::type::object},
    value        { enclosing,
                   (self || !enclosing) ? self
                                        : value::get<environment>(enclosing).self,
                   {}}
{ }

vm::call_frame::call_frame(vector_ref<vm::command> instr_ptr,
                           gc::managed_ptr env,
                           size_t argc,
                           size_t frame_ptr)
  : argc       {argc},
    frame_ptr  {frame_ptr},
    env        {env},
    caller     {},
    catcher    {},
    instr_ptr  {instr_ptr}
{ }
