#include "builtins.h"
#include "value/type.h"

vv::value::type vv::builtin::type::nil {[]{ return nullptr; }, {
}, vv::builtin::type::object, {"Nil"}};
