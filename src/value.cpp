#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "utils/lang.h"

using namespace vv;

value::object::object(struct type* new_type)
  : members  {},
    type     {new_type}
{ }

value::object::object()
  : members  {},
    type     {&builtin::type::object}
{ }

size_t value::object::hash() const
{
  const static std::hash<const void*> hasher{};
  return hasher(static_cast<const void*>(this));
}

bool value::object::equals(const value::object& other) const
{
  return this == &other;
}

void value::object::mark()
{
  if (type)
    gc::mark(type);
  for (auto& i : members)
    gc::mark(i.second);
}

value::basic_function::basic_function(func_type type,
                                      int argc,
                                      vm::environment* enclosing,
                                      vector_ref<vm::command> body)
  : object    {&builtin::type::function},
    type      {type},
    argc      {argc},
    enclosing {enclosing},
    body      {body}
{ }

void value::basic_function::mark()
{
  object::mark();
  if (enclosing)
    gc::mark(enclosing);
}

value::type::type(const std::function<object*()>& constructor,
                  const hash_map<vv::symbol, basic_function*>& methods,
                  type& parent,
                  vv::symbol name)
  : object      {&builtin::type::custom_type},
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

std::string value::type::value() const { return to_string(name); }

void value::type::mark()
{
  object::mark();
  for (const auto& i : methods)
    gc::mark(i.second);
  gc::mark(&parent);
}
