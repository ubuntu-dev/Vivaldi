#include "builtins.h"

#include "utils/lang.h"
#include "value/opt_functions.h"

using namespace vv;
using namespace builtin;

namespace {

value::object_ptr fn_custom_type_parent(value::object_ptr self)
{
  return static_cast<value::type&>(*self).parent;
}

value::opt_monop custom_type_parent {fn_custom_type_parent};

}
value::type type::custom_type {[]{ return nullptr; }, {
  { {"parent"}, &custom_type_parent }
}, &builtin::type::object, {"Type"}};
