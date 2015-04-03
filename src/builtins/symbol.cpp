#include "builtins.h"

#include "gc/alloc.h"
#include "messages.h"
#include "utils/lang.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/symbol.h"
#include "value/string.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

namespace {

const std::string& to_string(value::basic_object* boxed)
{
  return static_cast<const value::string&>(*boxed).val;
}

vv::symbol to_symbol(value::basic_object* boxed)
{
  return static_cast<const value::symbol&>(*boxed).val;
}

value::basic_object* fn_symbol_init(vm::machine& vm)
{
  vm.self();
  auto sym = static_cast<value::symbol*>(vm.top());
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type == &type::symbol)
    sym->val = to_symbol(arg);
  else if (arg->type == &type::string)
    sym->val = vv::symbol{to_string(arg)};
  else
    return throw_exception(message::init_multi_type_error(type::symbol, *arg->type));
  return sym;
}

value::basic_object* fn_symbol_equals(value::basic_object* self, value::basic_object* arg)
{
  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>(to_symbol(self) == to_symbol(arg));
}

value::basic_object* fn_symbol_unequal(value::basic_object* self, value::basic_object* arg)
{
  if (arg->type != &type::symbol)
    return gc::alloc<value::boolean>( true );
  return gc::alloc<value::boolean>(to_symbol(self) != to_symbol(arg));
}

value::builtin_function symbol_init    {fn_symbol_init, 1};
value::opt_binop        symbol_equals  {fn_symbol_equals };
value::opt_binop        symbol_unequal {fn_symbol_unequal};

}

value::type type::symbol {gc::alloc<value::symbol>, {
  { {"init"},    &symbol_init    },
  { {"equals"},  &symbol_equals  },
  { {"unequal"}, &symbol_unequal },
}, builtin::type::object, {"Symbol"}};
