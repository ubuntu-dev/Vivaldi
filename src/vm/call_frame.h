#ifndef VV_VM_CALL_FRAME_H
#define VV_VM_CALL_FRAME_H

#include "instruction.h"

#include "symbol.h"
#include "value.h"
#include "utils/dumb_ptr.h"
#include "utils/hash_map.h"
#include "utils/vector_ref.h"

namespace vv {

namespace vm {

class environment : public value::base {
public:
  environment(environment* enclosing = nullptr,
              value::base* self      = nullptr);

  // Frame in which current function (ie closure) was defined
  dumb_ptr<environment> enclosing;
  // self, if this is a method call
  dumb_ptr<value::base> self;

  void mark() override;
};

struct call_frame {
  call_frame(vector_ref<vm::command> instr_ptr = {},
             environment* env                  = nullptr,
             size_t argc                       = 0,
             size_t frame_ptr                  = 0);

  size_t argc;
  size_t frame_ptr;

  dumb_ptr<environment> env;

  dumb_ptr<value::base> caller;
  dumb_ptr<value::base> catcher;

  vector_ref<vm::command> instr_ptr;
};

}

}

#endif
