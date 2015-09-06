#ifndef VV_VALUE_FUNCTION_H
#define VV_VALUE_FUNCTION_H

#include "value/basic_object.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

struct function : public basic_object {
  function(int argc,
           const std::vector<vm::command>& body,
           gc::managed_ptr enclosure,
           bool takes_varargs = false);

  struct value_type {
    std::vector<vm::command> body;
    int argc;
    gc::managed_ptr enclosure;
    bool takes_varargs;
  };

  value_type value;
};

}

}

#endif
