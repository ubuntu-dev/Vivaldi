#ifndef VV_BUILTINS_RANGE_H
#define VV_BUILTINS_RANGE_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace range {

gc::managed_ptr init(vm::machine& vm);
gc::managed_ptr start(gc::managed_ptr self);
gc::managed_ptr size(vm::machine& vm);
gc::managed_ptr at_end(vm::machine& vm);
gc::managed_ptr get(gc::managed_ptr self);
gc::managed_ptr increment(vm::machine& vm);
gc::managed_ptr to_arr(vm::machine& vm);

}

}

}

#endif
