#ifndef VV_BUILTINS_STRING_H
#define VV_BUILTINS_STRING_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace string {

gc::managed_ptr init(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr size(gc::managed_ptr self);

gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less_equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater_equals(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr add(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr times(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr to_int(gc::managed_ptr self);
gc::managed_ptr to_flt(gc::managed_ptr self);

gc::managed_ptr to_sym(gc::managed_ptr self);

gc::managed_ptr at(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr start(gc::managed_ptr self);
gc::managed_ptr stop(gc::managed_ptr self);

gc::managed_ptr to_upper(gc::managed_ptr self);
gc::managed_ptr to_lower(gc::managed_ptr self);

gc::managed_ptr starts_with(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr ord(gc::managed_ptr self);

gc::managed_ptr split(vm::machine& vm);
gc::managed_ptr replace(vm::machine& vm);

}

namespace string_iterator {

gc::managed_ptr at_start(gc::managed_ptr self);
gc::managed_ptr at_end(gc::managed_ptr self);
gc::managed_ptr get(gc::managed_ptr self);

gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr increment(gc::managed_ptr self);
gc::managed_ptr decrement(gc::managed_ptr self);

gc::managed_ptr add(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr subtract(gc::managed_ptr self, gc::managed_ptr arg);

}

}

}

#endif
