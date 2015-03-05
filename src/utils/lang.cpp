#include "lang.h"

#include "builtins.h"
#include "gc.h"
#include "utils/error.h"
#include "value/boolean.h"
#include "value/string.h"

bool vv::truthy(const value::base* val)
{
  if (val->type == &builtin::type::nil)
    return false;
  else if (val->type == &builtin::type::boolean)
    return static_cast<const value::boolean*>(val)->val;
  return true;
}

[[noreturn]]
vv::value::base* vv::throw_exception(const std::string& value)
{
  throw vm_error{gc::alloc<value::string>( value )};
}

[[noreturn]]
vv::value::base* vv::throw_exception(value::base* value)
{
  throw vm_error{value};
}

vv::value::base* vv::find_method(value::type* t, symbol name)
{
  decltype(begin(t->methods)) i{};
  while ((i = t->methods.find(name)) == end(t->methods) && &t->parent != t)
    t = static_cast<value::type*>(&t->parent);

  if (i != end(t->methods))
    return i->second;

  return nullptr;
}
