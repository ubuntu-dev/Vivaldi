#include "lang.h"

#include "gc/alloc.h"
#include "utils/error.h"
#include "value/string.h"

bool vv::truthy(gc::managed_ptr val)
{
  if (val.tag() == tag::nil)
    return false;
  else if (val.tag() == tag::boolean)
    return value::get<value::boolean>(val);
  return true;
}

[[noreturn]]
vv::gc::managed_ptr vv::throw_exception(const std::string& value)
{
  throw vm_error{gc::alloc<value::string>( value )};
}

[[noreturn]]
vv::gc::managed_ptr vv::throw_exception(gc::managed_ptr value)
{
  throw vm_error{value};
}

std::string vv::pretty_print(gc::managed_ptr object, vm::machine& vm)
{
  if (get_method(object.type(), {"str"})) {
    vm.push(object);
    vm.method({"str"});
    vm.call(0);
    vm.run_cur_scope();
    const auto str = vm.top();
    vm.pop(1);
    return value_for(str);
  }

  return value_for(object);
}
