#ifndef VV_VALUE_TYPE_H
#define VV_VALUE_TYPE_H

#include "value.h"

#include "basic_object.h"
#include "utils/hash_map.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

// C++ representation of a Vivaldi Type.
struct type : public basic_object {
  type(const std::function<gc::managed_ptr()>& constructor,
       const hash_map<vv::symbol, gc::managed_ptr>& methods,
       gc::managed_ptr parent,
       vv::symbol name);

  struct value_type {
    // Class methods.
    // Whenever a basic_object's member is looked for, if it isn't found
    // locally, the class will search its type's methods, and that type's
    // parent's methods, and so on recursively until it's found or there are no
    // more parents left.
    hash_map<vv::symbol, gc::managed_ptr> methods;

    // Very simple constructor.
    // This constructor should just provides an allocated bit of memory of the
    // appropriate type; any actual initialization (including reading passed
    // arguments) has to happen in the class's "init" method.
    std::function<gc::managed_ptr()> constructor;

    // Parent class. Parent classes are unchangeable and can't ever be null
    // (Object's just points to itself).
    gc::managed_ptr parent;

    // Name of class. Stored in class so value() can be prettier than just
    // "<type>".
    vv::symbol name;
  };

  value_type value;
};

}

}

#endif
