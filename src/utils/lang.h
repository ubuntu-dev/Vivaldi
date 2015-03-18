#ifndef VV_UTILS_LANG_H
#define VV_UTILS_LANG_H

#include "value.h"
#include "vm.h"

namespace vv {

bool truthy(const value::object* value);

[[noreturn]]
value::object* throw_exception(const std::string& value);
[[noreturn]]
value::object* throw_exception(value::object* value);

value::object* find_method(value::type* type, symbol name);

std::string pretty_print(value::object& object, vm::machine& vm);

}

#endif
