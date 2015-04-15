#ifndef VV_BUILTINS_CHARACTER_H
#define VV_BUILTINS_CHARACTER_H

#include "gc/managed_ptr.h"

namespace vv {

namespace builtin {

namespace character {

gc::managed_ptr ord(gc::managed_ptr self);

gc::managed_ptr to_str(gc::managed_ptr self);

}

}

}

#endif
