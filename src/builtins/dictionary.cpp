#include "builtins/dictionary.h"

#include "builtins.h"
#include "gc/alloc.h"
#include "messages.h"
#include "utils/lang.h"
#include "value/dictionary.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr dictionary::init(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::dictionary) {
    return throw_exception(type::type_error,
                           message::init_type_error(type::dictionary,
                                                    type::dictionary,
                                                    arg.type()));
  }
  value::get<value::dictionary>(self) = value::get<value::dictionary>(arg);
  return self;
}

gc::managed_ptr dictionary::size(gc::managed_ptr self)
{
  const auto sz = value::get<value::dictionary>(self).size();
  return gc::alloc<value::integer>( static_cast<value::integer>(sz) );
}

gc::managed_ptr dictionary::at(gc::managed_ptr self, gc::managed_ptr arg)
{
  auto& dict = value::get<value::dictionary>(self);
  const auto& mem = dict.find(arg);
  if (mem == end(dict))
    return gc::alloc<value::nil>( );
  return mem->second;
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
