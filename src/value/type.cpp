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
    value             {methods, constructor, parent, name}
{ }
