#ifndef VV_UTILS_LANG_H
#define VV_UTILS_LANG_H

#include "vm.h"

namespace vv {

bool truthy(gc::managed_ptr value);

[[noreturn]]
gc::managed_ptr throw_exception(gc::managed_ptr type, const std::string& value);

std::string pretty_print(gc::managed_ptr object, vm::machine& vm);

}

#endif
