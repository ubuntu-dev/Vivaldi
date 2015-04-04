#include "dictionary.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::dictionary::dictionary(const value_type& mems)
  : basic_object {builtin::type::dictionary},
    value        {mems}
{ }
