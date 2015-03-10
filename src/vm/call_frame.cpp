#include "call_frame.h"

#include "gc.h"

using namespace vv;

vm::environment::environment(environment* new_enclosing,
                             value::base* new_self)
  : enclosing {new_enclosing},
    self      {new_self || !enclosing ? new_self : enclosing->self}
{ }

void vm::environment::mark()
{
  base::mark();

  if (enclosing && !enclosing->marked())
    gc::mark(*enclosing);

  if (self && !self->marked())
    gc::mark(*self);
}

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
