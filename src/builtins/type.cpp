#include "builtins/type.h"

#include "builtins.h"
#include "messages.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/opt_functions.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr custom_type::parent(gc::managed_ptr self)
{
  return value::get<value::type>(self).parent;
}

gc::managed_ptr custom_type::new_obj(vm::machine& vm)
{
  // get self and, set up arguments to be passed to init
  vm.self();
  const auto& self = vm.top();
  vm.varg(0);
  const auto& vargs = value::get<value::array>(vm.top());
  const auto init_argc = vargs.size();
  vm.pop(2);
  for_each(rbegin(vargs), rend(vargs), [&vm](const auto i) { vm.push(i); });

  // allocate new object
  auto ctor_type = self;
  while (!value::get<value::type>(ctor_type).constructor)
    ctor_type = value::get<value::type>(ctor_type).parent;

  const auto obj = value::get<value::type>(ctor_type).constructor();
  // nonconstructible types have 'constructors' that return nullptr
  if (!obj)
    return throw_exception(builtin::type::type_error,
                           message::nonconstructible(ctor_type));
  obj.get()->type = self;
  vm.push(obj);
  vm.opt_tmpm({"init"});
  vm.call(init_argc);
  vm.run_cur_scope();

  return obj;
}
