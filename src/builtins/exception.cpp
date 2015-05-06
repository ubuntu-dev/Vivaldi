#include "builtins/exception.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/exception.h"
#include "value/string.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr builtin::exception::init(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() == tag::string)
    value::get<value::exception>(self).message = value::get<value::string>(arg);
  else
    return throw_exception(message::init_multi_type_error(type::string, arg.type()));
  return self;
}

gc::managed_ptr builtin::exception::message(gc::managed_ptr self)
{
  return gc::alloc<value::string>( value::get<value::exception>(self).message );
}
