#ifndef C_INTERNAL_H
#define C_INTERNAL_H

#include "gc/managed_ptr.h"
#include "vivaldi.h"

namespace vv {

gc::managed_ptr cast_from(vv_object_t obj);

vv_object_t cast_to(gc::managed_ptr ptr);

}

#endif
