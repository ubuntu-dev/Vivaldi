#ifndef VV_BUILTINS_EXCEPTION_H
#define VV_BUILTINS_EXCEPTION_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace exception {

gc::managed_ptr init(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr message(gc::managed_ptr self);

}

}

}

#endif
