#ifndef VV_BUILTINS_REGEX_H
#define VV_BUILTINS_REGEX_H

#include "vm.h"

namespace vv {

namespace builtin {

namespace regex {

gc::managed_ptr init(vm::machine& vm);
gc::managed_ptr match(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr match_index(gc::managed_ptr self, gc::managed_ptr arg);

}

namespace regex_result {

gc::managed_ptr at(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr index(gc::managed_ptr self, gc::managed_ptr arg);
gc::managed_ptr size(gc::managed_ptr self);

}

}

}

#endif
