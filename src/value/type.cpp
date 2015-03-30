#include "type.h"

#include "builtins.h"
#include "utils/lang.h"

using namespace vv;

value::type::type(const std::function<object*()>& constructor,
                  const hash_map<vv::symbol, basic_function*>& methods,
                  type& parent,
                  vv::symbol name)
  : object      {&builtin::type::custom_type, tag::type},
    methods     {methods},
    constructor {constructor},
    parent      {parent},
    name        {name}
{
  if (auto init = find_method(*this, {"init"})) {
    init_shim.argc = init->argc;

    for (auto i = 0; i != init_shim.argc; ++i) {
      init_shim.body.emplace_back(vm::instruction::arg, i);
    }

    init_shim.body.emplace_back( vm::instruction::self );
    init_shim.body.emplace_back( vm::instruction::readm, vv::symbol{"init"} );
    init_shim.body.emplace_back( vm::instruction::call, init_shim.argc );
    init_shim.body.emplace_back( vm::instruction::self );
    init_shim.body.emplace_back( vm::instruction::ret, false );
  }
  else {
    init_shim.argc = 0;
    init_shim.body = {
      { vm::instruction::self },
      { vm::instruction::ret, false }
    };
  }
}
