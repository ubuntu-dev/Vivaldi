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

  void mark();

  void push_bool(bool val);
  void push_flt(double val);
  void push_fn(const function_t& val);
  void push_int(int val);
  void push_nil();
  void push_str(const std::string& val);
  void push_sym(symbol val);
  void push_type(const type_t& type);

  void make_arr(int size);
  void make_dict(int size);

  void read(symbol sym);
  void write(symbol sym);
  void let(symbol sym);

  void self();
  void push_arg();
  void arg(int idx);
  void readm(symbol sym);
  void writem(symbol sym);
  void call(int args);
  void new_obj(int args);

  void eblk();
  void lblk();
  void ret(bool copy);

  void push();
  void pop();

  void req(const std::string& file);

  void jmp(int offset);
  void jmp_false(int offset);
  void jmp_true(int offset);

  void push_catch();
  void pop_catch();
  void except();

  void chdir(const std::string& new_dir);

  value::base* retval;

private:
  void run_single_command(const vm::command& command);

  void except_until(size_t stack_pos);

  call_frame& frame();

  std::vector<call_frame> m_call_stack;
  std::vector<value::base*> m_stack;
  std::function<void(machine&)> m_exception_handler;

  value::base* m_pushed_self;
};

}

}

#endif
