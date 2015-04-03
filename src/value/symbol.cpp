#include "value/symbol.h"

#include "builtins.h"

using namespace vv;

value::symbol::symbol(vv::symbol new_val)
  : basic_object {&builtin::type::symbol, tag::symbol},
    val          {new_val}
{ }
