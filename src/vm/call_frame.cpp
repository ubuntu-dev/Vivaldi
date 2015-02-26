#include "call_frame.h"

using namespace vv;

vm::environment::environment(const std::shared_ptr<environment>& new_enclosing,
                             value::base* new_self)
  : enclosing {new_enclosing},
    self      {new_self || !enclosing ? new_self : enclosing->self}
{ }

void vm::mark(environment& frame)
{
  // Tedious; just call mark on every extant member (unless it's already marked,
  // in which case don't--- both because it's redundant and because of circular
  // references)
  if (frame.enclosing)
    mark(*frame.enclosing);

  for (auto& i : frame.local)
    if (!i.second->marked())
      i.second->mark();

  if (frame.self && !frame.self->marked())
    frame.self->mark();
}

vm::call_frame::call_frame(vector_ref<vm::command> instr_ptr,
                           const std::shared_ptr<environment>& env,
                           size_t argc,
                           size_t frame_ptr)
  : argc       {argc},
    frame_ptr  {frame_ptr},
    env        {env},
    caller     {nullptr},
    catcher    {nullptr},
    instr_ptr  {instr_ptr}
{ }
