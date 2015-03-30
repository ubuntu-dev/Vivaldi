#ifndef VV_VALUE_FUNCTION_H
#define VV_VALUE_FUNCTION_H

#include "value/basic_function.h"
#include "vm/call_frame.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

struct function : public basic_function {
  function(int argc,
           const std::vector<vm::command>& body,
           vm::environment* enclosure);

  std::vector<vm::command> vec_body;
};

}

}

#endif
