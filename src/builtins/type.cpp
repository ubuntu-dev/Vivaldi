#include "builtins.h"

#include "utils/lang.h"
#include "value/builtin_function.h"

using namespace vv;
using namespace builtin;

namespace {

value::base* fn_custom_type_parent(vm::machine& vm)
{
  vm.self();
  return &static_cast<value::type&>(*vm.retval).parent;
}

value::builtin_function custom_type_parent {fn_custom_type_parent, 0};

}
value::type type::custom_type {[]{ return nullptr; }, {
  { {"parent"}, &custom_type_parent }
}, builtin::type::object, {"Type"}};
