#ifndef VV_BUILTINS_SYMBOl_H
#define VV_BUILTINS_SYMBOl_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace symbol {

gc::managed_ptr init(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);

}

}

}

#endif
