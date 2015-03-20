#ifndef VV_VALUE_FUNCTION_H
#define VV_VALUE_FUNCTION_H

#include "value.h"
#include "vm/call_frame.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

struct function : public basic_function {
  function(int argc,
           const std::vector<vm::command>& body,
           gc::managed_ptr<vm::environment> enclosure);

  std::string value() const override;

  std::vector<vm::command> vec_body;
};

}

}

#endif
