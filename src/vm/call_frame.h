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

// Vivaldi's call 'stack' is very loosely based on Python's; it consists of a
// vector of shared_ptr<call_frame>'s (as defined below). Obviously this is
// pretty much worse in every respect than just a vector<call_frame>, but
// closures mess this up by possibly requiring access to frames no longer on the
// call stack. This approach is pretty slow, but unless a better means of
// managing closures presents iteslf we're stuck with it.

class call_frame {
public:
  call_frame(vector_ref<command>         instr_ptr = {},
             std::shared_ptr<call_frame> enclosing = nullptr,
             size_t                      args      = 0);

  // Frame in which current function (ie closure) was defined
  const std::shared_ptr<call_frame> enclosing;
  // Local variables
  std::vector<std::unordered_map<symbol, value::base*>> local;
  // self, if this is a method call
  dumb_ptr<value::base> self;

  // Number of function arguments --- stored in parent's pushed
  size_t args;
  // Position pointing to top of frame in VM stack
  size_t frame_ptr;

  // Catch expression provided by try...catch blocks
  dumb_ptr<value::base> catcher;

  // Current instruction pointer
  vector_ref<command> instr_ptr;

};

void mark(call_frame& frame);

}

}

#endif
