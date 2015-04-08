#include "builtins/symbol.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/symbol.h"
#include "value/string.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr builtin::symbol::init(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() == tag::symbol)
    value::get<value::symbol>(self) = value::get<value::symbol>(arg);
  else if (arg.tag() == tag::string)
    value::get<value::symbol>(self) = vv::symbol{value::get<value::string>(arg)};
  else
    return throw_exception(message::init_multi_type_error(type::symbol, arg.type()));
  return self;
}

gc::managed_ptr builtin::symbol::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::symbol)
    return gc::alloc<value::boolean>( false );

  return gc::alloc<value::boolean>(value::get<value::symbol>(self) ==
                                   value::get<value::symbol>(arg));
}

gc::managed_ptr builtin::symbol::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::symbol)
    return gc::alloc<value::boolean>( true );

  return gc::alloc<value::boolean>(value::get<value::symbol>(self) !=
                                   value::get<value::symbol>(arg));
}
