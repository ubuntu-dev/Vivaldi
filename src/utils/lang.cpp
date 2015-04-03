#include "lang.h"

#include "builtins.h"
#include "gc/alloc.h"
#include "utils/error.h"
#include "value/boolean.h"
#include "value/string.h"
#include "value/type.h"

bool vv::truthy(const value::basic_object& val)
{
  if (val.type == &builtin::type::nil)
    return false;
  else if (val.type == &builtin::type::boolean)
    return static_cast<const value::boolean&>(val).val;
  return true;
}

[[noreturn]]
vv::value::basic_object* vv::throw_exception(const std::string& value)
{
  throw vm_error{gc::alloc<value::string>( value )};
}

[[noreturn]]
vv::value::basic_object* vv::throw_exception(value::basic_object* value)
{
  throw vm_error{value};
}

std::string vv::pretty_print(value::basic_object* object, vm::machine& vm)
{
  if (has_member(*object, {"str"}) || get_method(*object->type, {"str"})) {
    vm.push(object);
    vm.readm({"str"});
    vm.call(0);
    vm.run_cur_scope();
    const auto str = vm.top();
    vm.pop(1);
    return value_for(*str);
  }

  return value_for(*object);
}
