#include "call_frame.h"

using namespace vv;

/*
vm::environment::environment(std::shared_ptr<environment> new_enclosing)
  : enclosing {new_enclosing},
    local     {{}}
{ }

void vm::mark(environment& frame)
{
  // Tedious; just call mark on every extant member (unless it's already marked,
  // in which case don't--- both because it's redundant and because of circular
  // references)
  if (frame.enclosing)
    mark(*frame.enclosing);

  for (auto& i : frame.local)
    for (auto& val : i)
      if (!val.second->marked())
        val.second->mark();

  if (frame.self && !frame.self->marked())
    frame.self->mark();
}
*/

vm::call_frame::call_frame(vector_ref<vm::command> instr_ptr,
                           call_frame* enclosing,
                           size_t argc,
                           size_t frame_ptr)
  : enclosing        {enclosing},
    argc             {argc},
    parent_frame_ptr {frame_ptr},
    local            {{}},
    caller           {nullptr},
    catcher          {nullptr},
    self             {nullptr},
    instr_ptr        {instr_ptr}
{ }

void vm::call_frame::mark()
{
  base::mark();

  if (enclosing && !enclosing->marked())
    enclosing->mark();

  for (auto& i : local)
    for (auto& val : i)
      if (!val.second->marked())
        val.second->mark();

  if (caller && !caller->marked())
    caller->mark();
  if (catcher && !catcher->marked())
    catcher->mark();
  if (self && !self->marked())
    self->mark();
}
