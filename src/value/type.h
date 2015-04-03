#ifndef VV_VALUE_TYPE_H
#define VV_VALUE_TYPE_H

#include "value.h"

#include "basic_object.h"
#include "basic_function.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

// C++ representation of a Vivaldi Type.
struct type : public basic_object {
  type(const std::function<basic_object*()>& constructor,
       const hash_map<vv::symbol, basic_function*>& methods,
       type& parent,
       vv::symbol name);

  // Class methods.
  // Whenever a basic_object's member is looked for, if it isn't found
  // locally, the class will search its type's methods, and that type's parent's
  // methods, and so on recursively until it's found or there are no more
  // parents left.
  hash_map<vv::symbol, basic_function*> methods;

  // Very simple constructor.
  // This constructor should just provides an allocated bit of memory of
  // the appropriate type; any actual initialization (including reading passed
  // arguments) has to happen in the class's "init" method.
  std::function<basic_object*()> constructor;

  // Blob of VM code used in initialization.
  // This shim is necessary because, of course, when you create a new object you
  // want to get that object back. Unfortunately it's not possible to guarantee
  // this in the init function, since someone could do something like
  //
  //     class Foo
  //       fn init(): 5
  //     end
  //
  // When you instantiate Foo, you'd prefer you get your new object back, not 5.
  // So the init shim creates a function that looks something like
  //
  //     fn (<args...): do
  //       self.init(<args...>)
  //       self
  //     end
  //
  // and the constructor calls that fake init function instead of 'init'.
  vm::function_t init_shim;

  // Parent class. Parent classes are stored as references, since they're
  // unchangeable and can't ever be null (basic_object's just points to itself).
  type& parent;

  // Name of class. Stored in class so value() can be prettier than just
  // "<type>".
  vv::symbol name;
};

}

}

#endif
