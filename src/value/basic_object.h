#ifndef VV_VALUE_BASIC_OBJECT_H
#define VV_VALUE_BASIC_OBJECT_H

#include "value.h"

namespace vv {

namespace value {

// Basic class from which all object types are derived.
struct basic_object {
  basic_object(gc::managed_ptr type);
  // Pointer to type
  gc::managed_ptr type;
};

}

}

#endif
