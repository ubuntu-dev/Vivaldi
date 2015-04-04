#include "builtins/dictionary.h"

#include "builtins.h"
#include "gc/alloc.h"
#include "messages.h"
#include "utils/lang.h"
#include "value/dictionary.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr dictionary::init(vm::machine& vm)
{
  vm.self();
  const auto dict = vm.top();
  vm.arg(0);
  const auto arg = vm.top();
  if (arg.tag() != tag::dictionary) {
    return throw_exception(message::init_type_error(type::dictionary,
                                                    type::dictionary,
                                                    arg.type()));
  }
  value::get<value::dictionary>(dict) = value::get<value::dictionary>(arg);
  return dict;
}

gc::managed_ptr dictionary::size(gc::managed_ptr self)
{
  const auto sz = value::get<value::dictionary>(self).size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

gc::managed_ptr dictionary::at(gc::managed_ptr self, gc::managed_ptr arg)
{
  auto& dict = value::get<value::dictionary>(self);
  if (!dict.count(arg))
    dict[arg] = gc::alloc<value::nil>( );
  return dict[arg];
}

gc::managed_ptr dictionary::set_at(vm::machine& vm)
{
  vm.self();
  auto& dict = value::get<value::dictionary>(vm.top());
  vm.arg(0);
  const auto arg = vm.top();
  vm.arg(1);
  return dict[arg] = vm.top();
}
