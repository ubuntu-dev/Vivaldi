#ifndef VV_VALUE_OBJECT_H
#define VV_VALUE_OBJECT_H

#include "value/basic_object.h"

#include "../symbol.h"
#include "utils/hash_map.h"

namespace vv {

namespace value {

// Represents a Vivaldi Object
struct object : public basic_object {
  // Creates a new value::object of type Object
  object();

  // Contains local, variable-specific members.
  // Only members of this specific object are stored here; methods are stored
  // inside of their owning classes.
  hash_map<vv::symbol, basic_object*> members;
};

}

}

#endif
