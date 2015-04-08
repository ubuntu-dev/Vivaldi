#ifndef VV_BUILTINS_FILE_H
#define VV_BUILTINS_FILE_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace file {

gc::managed_ptr init(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr contents(gc::managed_ptr self);
gc::managed_ptr start(gc::managed_ptr self);
gc::managed_ptr get(gc::managed_ptr self);
gc::managed_ptr increment(gc::managed_ptr increment);
gc::managed_ptr at_end(gc::managed_ptr at_end);

}

}

}

#endif
