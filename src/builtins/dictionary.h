#ifndef VV_BUILTINS_DICTIONARY_H
#define VV_BUILTINS_DICTIONARY_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace dictionary {

gc::managed_ptr init(vm::machine& vm);
gc::managed_ptr size(gc::managed_ptr self);
gc::managed_ptr at(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr set_at(vm::machine& vm);

}

}

}

#endif
