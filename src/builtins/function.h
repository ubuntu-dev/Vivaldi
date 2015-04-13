#ifndef VV_BUILTINS_FUNCTION_H
#define VV_BUILTINS_FUNCTION_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace function {

gc::managed_ptr bind(gc::managed_ptr self, gc::managed_ptr arg);

}

}

}

#endif
