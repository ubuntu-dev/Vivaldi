#ifndef VV_VM_H
#define VV_VM_H

#include "vm/call_frame.h"

namespace vv {

namespace vm {

class machine {
public:
  machine(call_frame&& frame,
          const std::function<void(vm::machine&)>& exception_handler);

  void run();
  void run_cur_scope();

  value::base* top();
  void push(value::base* newtop);

  void mark();

  void pbool(bool val);
  void pflt(double val);
  void pfn(const function_t& val);
  void pint(int val);
  void pnil();
  void pstr(const std::string& val);
  void psym(symbol val);
  void ptype(const type_t& type);

  void parr(int size);
  void pdict(int size);

  void read(symbol sym);
  void write(symbol sym);
  void let(symbol sym);

  void self();
  void arg(int idx);
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

  void chdir(const std::string& new_dir);

  void opt_add();
  void opt_sub();
  void opt_mul();
  void opt_div();

  void opt_not();

private:
  void run_single_command(const vm::command& command);

  void except_until(size_t stack_pos);

  call_frame& frame();

  std::vector<call_frame> m_call_stack;
  std::vector<value::base*> m_stack;

  value::base* m_transient_self;

  std::function<void(machine&)> m_exception_handler;
};

}

}

#endif
