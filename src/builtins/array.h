#ifndef VV_BUILTINS_ARRAY_H
#define VV_BUILTINS_ARRAY_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace array {

gc::managed_ptr init(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr size(gc::managed_ptr self);
gc::managed_ptr append(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr pop(gc::managed_ptr self);
gc::managed_ptr at(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr set_at(vm::machine& vm);
gc::managed_ptr start(gc::managed_ptr self);
gc::managed_ptr stop(gc::managed_ptr self);
gc::managed_ptr add(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr equals(vm::machine& vm);
gc::managed_ptr unequal(vm::machine& vm);

}

namespace array_iterator {

gc::managed_ptr at_start(gc::managed_ptr self);
gc::managed_ptr at_end(gc::managed_ptr self);
gc::managed_ptr get(gc::managed_ptr self);
gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr increment(gc::managed_ptr self);
gc::managed_ptr decrement(gc::managed_ptr self);
gc::managed_ptr add(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr subtract(gc::managed_ptr self, gc::managed_ptr arg);

}

}

}

#endif
