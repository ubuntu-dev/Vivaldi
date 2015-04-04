#ifndef VV_BUILTINS_INTEGER_H
#define VV_BUILTINS_INTEGER_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace integer {

gc::managed_ptr add(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr subtract(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr times(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr divides(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr modulo(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr pow(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr lshift(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr rshift(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr bit_and(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr bit_or(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr x_or(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr unequal(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr less_equals(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr greater_equals(gc::managed_ptr self, gc::managed_ptr arg);

gc::managed_ptr negative(gc::managed_ptr self);
gc::managed_ptr negate(gc::managed_ptr self);
gc::managed_ptr sqrt(gc::managed_ptr self);
gc::managed_ptr sin(gc::managed_ptr self);
gc::managed_ptr cos(gc::managed_ptr self);
gc::managed_ptr tan(gc::managed_ptr self);
gc::managed_ptr chr(gc::managed_ptr self);

}

}

}

#endif
