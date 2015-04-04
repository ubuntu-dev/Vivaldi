#ifndef VV_GC_H
#define VV_GC_H

#include "value.h"
#include "utils/dynamic_library.h"
#include "vm.h"

#include <array>

namespace vv {

namespace gc {

void set_running_vm(vm::machine& vm);
vm::machine& get_running_vm();

dynamic_library& load_dynamic_library(const std::string& filename);

void mark(gc::managed_ptr basic_object);

}

}

#endif
