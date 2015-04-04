#include "builtins/type.h"

#include "utils/lang.h"
#include "value/opt_functions.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr custom_type::parent(gc::managed_ptr self)
{
  return value::get<value::type>(self).parent;
}
