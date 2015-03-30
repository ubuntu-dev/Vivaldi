#ifndef VV_VALUE_OBJECT_H
#define VV_VALUE_OBJECT_H

#include "value.h"

#include "../symbol.h"
#include "utils/hash_map.h"

namespace vv {

namespace value {

// Basic Object class from which all types are derived.
struct object {
  // Creates a new value::object of the provided type.
  object(type* type, tag tag);
  // Creates a new value::object of type Object
  object();

  tag tag;

  // Contains local, variable-specific members.
  // Only members of this specific object are stored here; methods are stored
  // inside of their owning classes.
  hash_map<vv::symbol, object*> members;
  // Pointer to type (this should never be null. TODO: make reference)
  type* type;
};

}

}

#endif
