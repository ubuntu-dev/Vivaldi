#include "value/symbol.h"

#include "builtins.h"

using namespace vv;

value::symbol::symbol(vv::symbol val)
  : basic_object {builtin::type::symbol},
    value        {val}
{ }
