#ifndef VV_VM_CALL_FRAME_H
#define VV_VM_CALL_FRAME_H

#include "instruction.h"

#include "symbol.h"
#include "value/basic_object.h"
#include "utils/hash_map.h"
#include "utils/vector_ref.h"

namespace vv {

namespace vm {

// Class representing a local execution environment.
//
// Each call frame has its own environment. If this is a closure (and all
// Vivaldi functions are closures) or a block, the enclosing environment is
// stored in environment::enclosing.
//
// Execution environments are, hackishly, actually Vivaldi basic_objects (they
// inherit from value::basic_object); this is because, because of closures, manual
// memory management is basically impossible, and even my simplistic garbage
// collector is significantly faster than reference counting via shared_ptr.
struct environment : public value::basic_object {
  environment(gc::managed_ptr enclosing = {},
              gc::managed_ptr self      = {});

  struct value_type {
    // Frame in which current function (i.e. closure) was defined.
    gc::managed_ptr enclosing;
    // self, if this is a method call.
    gc::managed_ptr self;
    hash_map<symbol, gc::managed_ptr> members;
  };

  value_type value;
};

// A single call frame. The VM's call stack is implemented as a vector of these
// in vm::machine. Each one represents a single function call.
struct call_frame {
  call_frame(vector_ref<vm::command> instr_ptr = {},
             gc::managed_ptr env               = {},
             size_t argc                       = 0,
             size_t frame_ptr                  = 0);

  // The numer of arguments the called function takes (used in stack
  // manipulation)
  size_t argc;
  // The position in the VM stack were local variables begin.
  size_t frame_ptr;

  // The outermost environment. Given lexical scoping, this will of course be
  // different for each call frame.
  gc::managed_ptr env;

  // The calling function, if there is one; stored here only for GC purposes.
  gc::managed_ptr caller;

  // The local catch function, if we're in a try...catch block.
  gc::managed_ptr catcher;

  // Pointer to the current VM instruction we're executing.
  vector_ref<vm::command> instr_ptr;
};

}

}

#endif
