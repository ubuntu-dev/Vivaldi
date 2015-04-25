#ifndef VV_VM_H
#define VV_VM_H

#include "vm/call_frame.h"

namespace vv {

namespace vm {

// Class implementing Vivaldi's virtual machine.
class machine {
public:
  machine(call_frame&& frame);

  machine(machine&& other) = delete;
  machine(const machine& other) = delete;
  machine& operator=(machine&& other) = delete;
  machine& operator=(const machine& other) = delete;

  // Run the current code until completion.
  void run();
  // Run the current code until we attempt to exit (or except out of) the call
  // frame on top of the call stack at the time run_cur_scope, at which point
  // control is returned to the calling C++ function.
  void run_cur_scope();

  // Returns the value on top of the stack.
  gc::managed_ptr top();
  // Pushes the provided value onto the stack.
  void push(gc::managed_ptr newtop);

  // GC interface; mark all basic_objects immediately reachable from within the VM.
  void mark();

  // VM Instructions (publicly accessible, since value::builtin_funciton needs
  // to be able to manipulate the VM). Documentation for all the instructions is
  // in vm/instruction.h.

  void pbool(bool val);
  void pchar(int val);
  void pflt(double val);
  void pfn(const function_t& val);
  void pint(int val);
  void pnil();
  void pstr(const std::string& val);
  void psym(symbol val);
  void ptype(const type_t& type);

  void pre(const std::string& val);

  void parr(int size);
  void pdict(int size);

  void read(symbol sym);
  void write(symbol sym);
  void let(symbol sym);

  void self();
  void arg(int idx);
  void method(symbol sym);
  void readm(symbol sym);
  void writem(symbol sym);
  void call(int args);
  void pobj(int args);

  void dup();
  void pop(int num);

  void eblk();
  void lblk();
  void ret(bool copy);

  void req(const std::string& file);

  void jmp(int offset);
  void jf(int offset);
  void jt(int offset);

  void pushc();
  void popc();
  void exc();

  void chreqp(const std::string& new_path);

  void noop();

  // Optimization VM instructions.

  void opt_tmpm(symbol name);

  void opt_add();
  void opt_sub();
  void opt_mul();
  void opt_div();

  void opt_not();

  void opt_get();
  void opt_at_end();
  void opt_incr();

  void opt_size();

private:
  void run_single_command(const vm::command& command);

  void except_until(size_t stack_pos);

  call_frame& frame();

  std::vector<call_frame> m_call_stack;
  std::vector<gc::managed_ptr> m_stack;

  gc::managed_ptr m_transient_self;

  std::string m_req_path;
};

}

}

#endif
