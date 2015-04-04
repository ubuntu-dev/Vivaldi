#include "type.h"

#include "builtins.h"
#include "value/builtin_function.h"
#include "value/function.h"

using namespace vv;

value::type::type(const std::function<gc::managed_ptr()>& constructor,
                  const hash_map<vv::symbol, gc::managed_ptr>& methods,
                  gc::managed_ptr parent,
                  vv::symbol name)
  : basic_object      {builtin::type::custom_type},
    value             {methods, constructor, {}, parent, name}
{
  auto init = value.methods[{"init"}];
  if (!init && parent)
    init = get_method(parent, {"init"});

  if (init) {
    if (init.tag() == tag::function) {
      value.init_shim.argc = get<function>(init).argc;
    }
    else if (init.tag() == tag::builtin_function) {
      value.init_shim.argc = get<builtin_function>(init).argc;
    }
    else if (init.tag() == tag::opt_monop) {
      value.init_shim.argc = 0;
    }
    else if (init.tag() == tag::opt_binop) {
      value.init_shim.argc = 1;
    }

    for (auto i = 0; i != value.init_shim.argc; ++i) {
      value.init_shim.body.emplace_back(vm::instruction::arg, i);
    }

    value.init_shim.body.emplace_back( vm::instruction::self );
    value.init_shim.body.emplace_back( vm::instruction::readm, vv::symbol{"init"} );
    value.init_shim.body.emplace_back( vm::instruction::call, value.init_shim.argc );
    value.init_shim.body.emplace_back( vm::instruction::self );
    value.init_shim.body.emplace_back( vm::instruction::ret, false );
  }
  else {
    value.init_shim.argc = 0;
    value.init_shim.body = {
      { vm::instruction::self },
      { vm::instruction::ret, false }
    };
  }
}
