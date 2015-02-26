#ifndef VV_VALUE_FUNCTION_H
#define VV_VALUE_FUNCTION_H

#include "value.h"
#include "vm/call_frame.h"
#include "vm/instruction.h"

namespace vv {

namespace value {

struct function : public base {
  function(int argc,
           const std::vector<vm::command>& body,
           vm::environment* enclosure);

  std::string value() const override;
  void mark() override;

  int argc;
  std::vector<vm::command> body;
  vm::environment* enclosure;
};

}

}

#endif
