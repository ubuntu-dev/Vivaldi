#ifndef VV_BUILTINS_FLOATING_POINT_H
#define VV_BUILTINS_FLOATING_POINT_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace floating_point {

gc::managed_ptr add(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr subtract(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr times(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr divides(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr pow(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less_equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater_equals(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr to_int(gc::managed_ptr self);

gc::managed_ptr negative(gc::managed_ptr self);
gc::managed_ptr sqrt(gc::managed_ptr self);
gc::managed_ptr sin(gc::managed_ptr self);
gc::managed_ptr cos(gc::managed_ptr self);
gc::managed_ptr tan(gc::managed_ptr self);

}

}

}

#endif
