#ifndef VV_UTILS_LANG_H
#define VV_UTILS_LANG_H

#include "value.h"
#include "vm.h"

namespace vv {

bool truthy(gc::managed_ptr<const value::object> value);

[[noreturn]]
value::object_ptr throw_exception(const std::string& value);
[[noreturn]]
value::object_ptr throw_exception(value::object_ptr value);

gc::managed_ptr<value::basic_function> find_method(gc::managed_ptr<value::type> type,
                                                   symbol name);

std::string pretty_print(value::object_ptr object, vm::machine& vm);

}

#endif
