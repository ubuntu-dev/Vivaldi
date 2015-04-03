#ifndef VV_UTILS_LANG_H
#define VV_UTILS_LANG_H

#include "value/basic_function.h"
#include "vm.h"

namespace vv {

bool truthy(const value::basic_object& value);

[[noreturn]]
value::basic_object* throw_exception(const std::string& value);
[[noreturn]]
value::basic_object* throw_exception(value::basic_object* value);

std::string pretty_print(value::basic_object* basic_object, vm::machine& vm);

}

#endif
