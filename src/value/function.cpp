#include "function.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::function::function(int new_argc,
                          const std::vector<vm::command>& new_body,
                          vm::environment* new_enclosing)
  : argc      {new_argc},
    body      {new_body},
    enclosing {new_enclosing}
{ }

std::string value::function::value() const { return "<function>"; }

int value::function::get_argc() const { return argc; }

vector_ref<vm::command> value::function::get_body() const { return body; }

vm::environment* value::function::get_enclosing() const { return enclosing; }

void value::function::mark()
{
  base::mark();
  if (enclosing && !enclosing->marked())
    enclosing->mark();
}
