#ifndef VV_VALUE_BASIC_OBJECT_H
#define VV_VALUE_BASIC_OBJECT_H

#include "value.h"

namespace vv {

namespace value {

// Basic class from which all object types are derived.
struct basic_object {
  basic_object(type* type, tag tag);

  tag tag;
  // Pointer to type (this should never be null. TODO: make reference)
  type* type;
};

}

}

#endif
