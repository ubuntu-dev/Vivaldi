#ifndef VV_VM_CALL_FRAME_H
#define VV_VM_CALL_FRAME_H

#include "instruction.h"

#include "symbol.h"
#include "value.h"
#include "utils/dumb_ptr.h"
#include "utils/vector_ref.h"

#include <unordered_map>

namespace vv {

namespace vm {

/*
class environment {
public:
  environment(std::shared_ptr<environment> enclosing = nullptr);

  // Frame in which current function (ie closure) was defined
  const std::shared_ptr<environment> enclosing;
  // Local variables
  // self, if this is a method call
};
*/

//void mark(environment& env);

struct call_frame : public value::base {
  const static size_t npos{std::numeric_limits<size_t>::max()};

  call_frame(vector_ref<vm::command> instr_ptr = {},
             call_frame* enclosing             = nullptr,
             size_t argc                       = 0,
             size_t frame_ptr                  = npos);

  call_frame* enclosing;

  size_t argc;
  size_t parent_frame_ptr;

  std::vector<std::unordered_map<symbol, value::base*>> local;

  dumb_ptr<value::base> caller;
  dumb_ptr<value::base> catcher;
  dumb_ptr<value::base> self;

  vector_ref<vm::command> instr_ptr;

  void mark() override;
};

}

}

#endif
