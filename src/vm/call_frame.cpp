#include "call_frame.h"

using namespace vv;

vm::call_frame::call_frame(vector_ref<command>         new_instr_ptr,
                           std::shared_ptr<call_frame> new_enclosing,
                           size_t                      new_args)
  : enclosing {new_enclosing},
    local     {{}},
    args      {new_args},
    instr_ptr {new_instr_ptr}
{ }

void vm::mark(call_frame& frame)
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

  if (frame.catcher && !frame.catcher->marked())
    frame.catcher->mark();
}
