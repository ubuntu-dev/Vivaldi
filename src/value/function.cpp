#include "function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::function::function(int new_argc,
                          const std::vector<vm::command>& new_body,
                          vm::environment* new_enclosure)
  : base      {&builtin::type::function},
    argc      {new_argc},
    body      {new_body},
    enclosure {new_enclosure}
{ }

std::string value::function::value() const { return "<function>"; }

void value::function::mark()
{
  base::mark();
  if (enclosure && !enclosure->marked())
    enclosure->mark();
}
