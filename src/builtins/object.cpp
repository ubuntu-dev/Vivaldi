#include "builtins/object.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/symbol.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr object::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return gc::alloc<value::boolean>( vv::equals(self, arg) );
}

gc::managed_ptr object::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  return gc::alloc<value::boolean>( !vv::equals(self, arg) );
}

gc::managed_ptr object::op_not(gc::managed_ptr self)
{
  return gc::alloc<value::boolean>( !truthy(self) );
}

gc::managed_ptr object::type(gc::managed_ptr self)
{
  return self.type();
}

gc::managed_ptr object::member(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::symbol) {
    return throw_exception(type::type_error,
                           message::type_error(type::symbol, arg.type()));
  }
  const auto mem = get_member(self, value::get<value::symbol>(arg));
  if (mem) {
    return mem;
  }
  return throw_exception(type::name_error,
                         "Member variable " +
                         to_string(value::get<value::symbol>(arg)) +
                         " does not exist");
}

gc::managed_ptr object::has_member(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::symbol) {
    return throw_exception(type::type_error,
                           message::type_error(type::symbol, arg.type()));
  }
  const auto mem = get_member(self, value::get<value::symbol>(arg));
  return gc::alloc<value::boolean>( mem ? true : false );
}

gc::managed_ptr object::set_member(vm::machine& vm)
{
  vm.self();
  const auto self = vm.top();
  vm.pop(1);
  vm.arg(0);
  const auto name = vm.top();
  vm.pop(1);
  vm.arg(1);
  const auto value = vm.top();
  vm.pop(1);
  if (name.tag() != tag::symbol) {
    return throw_exception(type::type_error,
                           message::type_error(type::symbol, name.type()));
  }
  set_member(self, value::get<value::symbol>(name), value);
  return self;
}
