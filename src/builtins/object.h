#ifndef VV_BUILTINS_OBJECT_H
#define VV_BUILTINS_OBJECT_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace object {

gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr op_not(gc::managed_ptr self);
gc::managed_ptr type(gc::managed_ptr self);

}

}

}

#endif
