#include "value/symbol.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::symbol::symbol(vv::symbol new_val)
  : object {&builtin::type::symbol, tag::symbol},
    val    {new_val}
{ }
