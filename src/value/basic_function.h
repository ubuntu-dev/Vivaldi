#ifndef VV_VALUE_BASIC_FUNCTION_H
#define VV_VALUE_BASIC_FUNCTION_H

#include "value/object.h"

#include "vm/call_frame.h"

namespace vv {

namespace value {

// Base class for all callable types. All function types (currently function,
// builtin_function, opt_monop, and opt_binop) are subclasses of basic_function.
struct basic_function : public object {
  basic_function(vv::tag type,
                 int argc,
                 vm::environment* enclosing,
                 vector_ref<vm::command> body);

  // Expected number of arguments.
  const int argc;
  // Enclosing environment, or nullptr if none exists.
  vm::environment* const enclosing;
  // The VM code to run in the new call frame (if this is a C++ function (as in
  // opt_monop, opt_binop, and builtin_function), just provide a stub with a
  // single 'ret false' command.
  vector_ref<vm::command> body;
};

}

}

#endif
