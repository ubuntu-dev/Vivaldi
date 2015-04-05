#include "vm.h"

#include "builtins.h"
#include "gc.h"
#include "get_file_contents.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/error.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/dictionary.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/opt_functions.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/symbol.h"
#include "value/type.h"

using namespace vv;

vm::machine::machine(call_frame&& frame)
  : m_call_stack     {frame},
    m_transient_self {},
    m_req_path       {""}
{
  // TODO: Merge VM and GC so they don't have to interact so weirdly
  gc::set_running_vm(*this);
}

void vm::machine::run()
{
  while (frame().instr_ptr.size()) {
    // Get next instruction (and argument, if it exists), and increment the
    // instruction pointer
    const auto& command = frame().instr_ptr.front();
    frame().instr_ptr = frame().instr_ptr.subvec(1);
    run_single_command(command);
  }
}

// Run VM until it attempts (via exception or via 'ret') to pop the current
// frame off the call stack
void vm::machine::run_cur_scope()
{
  const auto exit_sz = m_call_stack.size();

  while (frame().instr_ptr.size()) {
    // Get next instruction (and argument, if it exists), and increment the
    // instruction pointer
    const auto& command = frame().instr_ptr.front();
    frame().instr_ptr = frame().instr_ptr.subvec(1);

    if (command.instr == vm::instruction::ret && m_call_stack.size() == exit_sz) {
      ret(command.arg.as_bool());
      return;
    }
    else if (command.instr == vm::instruction::exc) {
      except_until(exit_sz);
    }
    else {
      run_single_command(command);
    }
  }
}

gc::managed_ptr vm::machine::top()
{
  return m_stack.back();
}

void vm::machine::push(gc::managed_ptr val)
{
  m_stack.push_back(val);
}

void vm::machine::mark()
{
  for (auto i : m_stack)
    gc::mark(i);

  gc::mark(m_transient_self);

  for (auto& i : m_call_stack) {
    gc::mark(i.caller);
    gc::mark(i.catcher);
    gc::mark(i.env);
  }
}

// Instruction implementations {{{

void vm::machine::pbool(bool val)
{
  push(gc::alloc<value::boolean>( val ));
}

void vm::machine::pflt(double val)
{
  push(gc::alloc<value::floating_point>( val ));
}

void vm::machine::pfn(const function_t& val)
{
  push(gc::alloc<value::function>(val.argc, val.body, frame().env));
}

void vm::machine::pint(int val)
{
  push(gc::alloc<value::integer>( val ));
}

void vm::machine::pnil()
{
  push(gc::alloc<value::nil>( ));
}

void vm::machine::pstr(const std::string& val)
{
  push(gc::alloc<value::string>( val ));
}

void vm::machine::psym(symbol val)
{
  push(gc::alloc<value::symbol>( val ));
}

void vm::machine::ptype(const type_t& type)
{
  read(type.parent);
  const auto parent_arg = top();
  if (parent_arg.tag() != tag::type) {
    pstr(message::inheritance_type_err);
    exc();
    return;
  }
  const auto parent = parent_arg;

  hash_map<symbol, gc::managed_ptr> methods;
  for (const auto& i : type.methods) {
    pfn(i.second);
    methods.insert(i.first, top());
  }
  const auto newtype = gc::alloc<value::type>(nullptr, methods, parent, type.name);

  pop(methods.size() + 1); // methods and parent
  push(newtype);
  let(type.name);
}

void vm::machine::pre(const std::string& val)
{
  try {
    push(gc::alloc<value::regex>( std::regex{val}, val ));
  } catch (const std::regex_error& e) {
    pstr(message::invalid_regex(e.what()));
    exc();
  }
}

void vm::machine::parr(const int size)
{
  std::vector<gc::managed_ptr> vec{end(m_stack) - size, end(m_stack)};
  const auto val = gc::alloc<value::array>( move(vec) );
  pop(size);
  push(val);
}

void vm::machine::pdict(const int size)
{
  value::dictionary::value_type dict;
  for (auto i = end(m_stack) - size; i != end(m_stack); i += 2)
    dict[i[0]] = i[1];

  const auto val = gc::alloc<value::dictionary>( dict );
  pop(size);
  push(val);
}

void vm::machine::read(const symbol sym)
{
  for (auto i = frame().env; i; i = value::get<environment>(i).enclosing) {
    const auto iter = value::get<environment>(i).members.find(sym);
    if (iter != std::end(value::get<environment>(i).members)) {
      push(iter->second);
      return;
    }
  }
  pstr(message::no_such_variable(sym));
  exc();
}

void vm::machine::write(const symbol sym)
{
  for (auto i = frame().env; i; i = value::get<environment>(i).enclosing) {
    const auto iter = value::get<environment>(i).members.find(sym);
    if (iter != std::end(value::get<environment>(i).members)) {
      iter->second = top();
      return;
    }
  }
  pstr(message::no_such_variable(sym));
  exc();
}

void vm::machine::let(const symbol sym)
{
  if (value::get<environment>(frame().env).members.count(sym)) {
    pstr(message::already_exists(sym));
    exc();
  }
  else {
    value::get<environment>(frame().env).members.insert(sym, top());
  }
}

void vm::machine::self()
{
  if (value::get<environment>(frame().env).self) {
    push(value::get<environment>(frame().env).self);
  }
  else {
    pstr(message::invalid_self_access);
    exc();
  }
}

void vm::machine::arg(const int idx)
{
  push(*(begin(m_stack) + frame().frame_ptr - idx));
}

void vm::machine::readm(const symbol sym)
{
  m_transient_self = top();
  pop(1);

  // First check local members, then methods
  if (has_member(m_transient_self, sym)) {
    push(get_member(m_transient_self, sym));
  }
  else if (auto method = get_method(m_transient_self.type(), sym)) {
    push(method);
  }
  else {
    pstr(message::has_no_member(m_transient_self, sym));
    exc();
  }
}

void vm::machine::writem(const symbol sym)
{
  const auto obj = top();
  m_stack.pop_back();
  set_member(obj, sym, top());
}

void vm::machine::call(const int argc)
{
  const static std::array<vm::command, 1> body_shim{{
    { instruction::ret, false }
  }};

  if (top().tag() != tag::function && top().tag() != tag::builtin_function &&
      top().tag() != tag::opt_monop && top().tag() != tag::opt_binop) {
    pstr(message::not_callable(top()));
    exc();
    return;
  }

  const auto func = top();

  try {
    if (func.tag() == tag::opt_monop) {
      if (argc != 0) {
        pstr(message::wrong_argc(0, argc));
        exc();
        return;
      }

      m_call_stack.push_back({body_shim, {}, 0, m_stack.size() - 2});
      frame().caller = func;
      pop(1); // func
      const auto ret = value::get<value::opt_monop>(func).body(m_transient_self);
      push(ret);

    }
    else if (func.tag() == tag::opt_binop) {
      if (argc != 1) {
        pstr(message::wrong_argc(1, argc));
        exc();
        return;
      }
      m_call_stack.push_back({body_shim, {}, 1, m_stack.size() - 2});
      frame().caller = func;
      pop(1); // func
      const auto ret = value::get<value::opt_binop>(func).body(m_transient_self,
                                                               top());
      push(ret);

    }
    else if (func.tag() == tag::builtin_function) {
      const auto expected = value::get<value::builtin_function>(func).argc;
      if (expected != static_cast<unsigned>(argc)) {
        pstr(message::wrong_argc(expected, argc));
        exc();
        return;
      }
      m_call_stack.push_back({body_shim, {}, expected, m_stack.size() - 2});
      frame().caller = func;
      pop(1); // func
      if (m_transient_self) {
        frame().env = gc::alloc<environment>( gc::managed_ptr{}, m_transient_self );
      }
      push(value::get<value::builtin_function>(func).body(*this));

    }
    else {
      const auto expected = value::get<value::function>(func).argc;
      if (expected != argc) {
        pstr(message::wrong_argc(expected, argc));
        exc();
        return;
      }
      m_call_stack.emplace_back(value::get<value::function>(func).body,
                                gc::alloc<environment>( value::get<value::function>(func).enclosure,
                                                        m_transient_self ),
                                expected,
                                m_stack.size() - 2);
      frame().caller = func;
      pop(1); // func
    }
  } catch (const vm_error& err) {
    push(err.error());
    exc();
  }
}

void vm::machine::pobj(const int argc)
{
  if (top().tag() != tag::type) {
    pstr(message::construction_type_err);
    exc();
    return;
  }

  const auto type = top();

  auto ctor_type = type;
  while (!value::get<value::type>(ctor_type).constructor)
    ctor_type = value::get<value::type>(ctor_type).parent;
  pop(1);
  push(value::get<value::type>(ctor_type).constructor());
  // Hack--- nonconstructible types (e.g. Integer) have constructors that return
  // nullptr
  if (!top()) {
    pop(1);
    pstr(message::nonconstructible(ctor_type));
    exc();
    return;
  }
  static_cast<value::basic_object*>(top().get())->type = type;
  m_transient_self = top();
  pop(1);
  pfn(value::get<value::type>(type).init_shim);
  call(argc);
}

void vm::machine::dup()
{
  push(top());
}

void vm::machine::pop(const int quant)
{
  m_stack.erase(end(m_stack) - quant, end(m_stack));
}

void vm::machine::eblk()
{
  frame().env = gc::alloc<environment>( frame().env );
}

void vm::machine::lblk()
{
  frame().env = value::get<environment>(frame().env).enclosing;
}

void vm::machine::ret(const bool copy)
{
  const auto retval = top();
  m_stack.erase(begin(m_stack) + frame().frame_ptr - frame().argc + 1, end(m_stack));
  push(retval);

  const auto cur_env = frame().env;
  m_call_stack.pop_back();
  if (copy)
    for (const auto& i : value::get<environment>(cur_env).members)
      value::get<environment>(frame().env).members[i.first] = i.second;
}

void vm::machine::req(const std::string& filename)
{
  // Add extension, if it was left off
  const auto name = get_real_filename(filename, m_req_path);
  if (is_c_exension(name)) {
    // Place in separate environment (along with environment) so as to avoid any
    // weirdnesses with adding things to the stack or declaring new variables
    m_call_stack.emplace_back(vector_ref<command>{},
                              gc::alloc<environment>( gc::managed_ptr{} ),
                              0,
                              m_stack.size() - 1);
    // TODO: Support actual error handling for C extensions
    const auto err = read_c_lib(name);
    if (err) {
      pstr("Unable to load C extension: " + *err);
      exc();
      return;
    }
    pnil();
    ret(true);
  }
  else {

    auto contents = get_file_contents(name, m_req_path);
    if (!contents.successful()) {
      pstr(contents.error());
      exc();
      return;
    }
    contents.result().emplace_back(instruction::pnil);
    contents.result().emplace_back(instruction::ret, true);
    pfn(function_t{0, contents.result()});
    call(0);
  }
}

void vm::machine::jmp(const int offset)
{
  frame().instr_ptr = frame().instr_ptr.shifted_by(offset);
}

void vm::machine::jf(const int offset)
{
  if (!truthy(top()))
    jmp(offset);
}

void vm::machine::jt(const int offset)
{
  if (truthy(top()))
    jmp(offset);
}

void vm::machine::pushc()
{
  frame().catcher = top();
  pop(1);
}

void vm::machine::popc()
{
  frame().catcher = {};
}

void vm::machine::exc()
{
  except_until(1);
}

void vm::machine::chreqp(const std::string& path)
{
  m_req_path = path;
}

void vm::machine::noop() { }

// }}}
// Optimizations {{{

namespace {

template <typename F>
void int_optimization(vm::machine& vm, const F& fn, const vv::symbol sym)
{
  const auto first = vm.top();
  vm.pop(1);
  const auto second = vm.top();

  if (first.tag() == tag::integer && !has_member(first, sym)) {
    if (second.tag() == tag::integer) {
      vm.pop(1);
      const auto left = value::get<value::integer>(first);
      const auto right = value::get<value::integer>(second);
      vm.pint(fn(left, right));
      return;
    }
  }
  vm.push(first);
  vm.readm(sym);
  vm.call(1);
  vm.run_cur_scope();
}

}

void vm::machine::opt_add()
{
  int_optimization(*this, std::plus<int>{}, builtin::sym::add);
}

void vm::machine::opt_sub()
{
  int_optimization(*this, std::minus<int>{}, builtin::sym::subtract);
}

void vm::machine::opt_mul()
{
  int_optimization(*this, std::multiplies<int>{}, builtin::sym::times);
}

void vm::machine::opt_div()
{
  int_optimization(*this, std::divides<int>{}, builtin::sym::divides);
}

void vm::machine::opt_not()
{
  const static symbol sym{"not"};

  const auto val = top();
  if (!has_member(val, sym)) {
    if (get_method(val.type(), sym) ==
        value::get<value::type>(builtin::type::object).methods.at(sym)) {
      const auto res = !truthy(val);
      pop(1);
      pbool(res);
      return;
    }
  }
  readm(sym);
  call(0);
  run_cur_scope();
}

// }}}

void vm::machine::run_single_command(const vm::command& command)
{
  using boost::get;

  const auto instr = command.instr;
  const auto& arg = command.arg;

  // HACK--- avoid weirdness like the following:
  //   let i = 1
  //   let add = i.add // self is now i
  //   add(2)          // => 3
  //   5 + 1           // self is now 5
  //   add(2)          // => 7
  if (instr != instruction::call)
    m_transient_self = {};

  switch (instr) {
  case instruction::pbool: pbool(arg.as_bool());  break;
  case instruction::pflt:  pflt(arg.as_double()); break;
  case instruction::pfn:   pfn(arg.as_fn());      break;
  case instruction::pint:  pint(arg.as_int());    break;
  case instruction::pnil:  pnil();                break;
  case instruction::pstr:  pstr(arg.as_str());    break;
  case instruction::psym:  psym(arg.as_sym());    break;
  case instruction::ptype: ptype(arg.as_type());  break;

  case instruction::pre: pre(arg.as_str()); break;

  case instruction::parr:  parr(arg.as_int());  break;
  case instruction::pdict: pdict(arg.as_int()); break;

  case instruction::read:  read(arg.as_sym());  break;
  case instruction::write: write(arg.as_sym()); break;
  case instruction::let:   let(arg.as_sym());   break;

  case instruction::self:   self();                  break;
  case instruction::arg:    this->arg(arg.as_int()); break;
  case instruction::readm:  readm(arg.as_sym());     break;
  case instruction::writem: writem(arg.as_sym());    break;
  case instruction::call:   call(arg.as_int());      break;
  case instruction::pobj:   pobj(arg.as_int());      break;

  case instruction::eblk: eblk();             break;
  case instruction::lblk: lblk();             break;
  case instruction::ret:  ret(arg.as_bool()); break;

  case instruction::dup: dup();             break;
  case instruction::pop: pop(arg.as_int()); break;

  case instruction::req: req(arg.as_str()); break;

  case instruction::jmp: jmp(arg.as_int()); break;
  case instruction::jf:  jf(arg.as_int());  break;
  case instruction::jt:  jt(arg.as_int());  break;

  case instruction::pushc: pushc(); break;
  case instruction::popc:  popc();  break;
  case instruction::exc:   exc();   break;

  case instruction::chreqp: chreqp(arg.as_str()); break;

  case instruction::noop: break;

  case instruction::opt_add: opt_add(); break;
  case instruction::opt_sub: opt_sub(); break;
  case instruction::opt_mul: opt_mul(); break;
  case instruction::opt_div: opt_div(); break;

  case instruction::opt_not: opt_not(); break;
  }
}

void vm::machine::except_until(const size_t stack_pos)
{
  const auto except_val = top();

  const auto last = find_if(rbegin(m_call_stack), rend(m_call_stack) - stack_pos,
                            [](const auto& i) { return i.catcher; });

  if (last != rbegin(m_call_stack)) {
    const auto last_erased = last.base();
    m_stack.erase(begin(m_stack) + last_erased->frame_ptr - last_erased->argc + 1,
                  end(m_stack));
  }
  m_call_stack.erase(last.base(), end(m_call_stack));
  push(except_val);


  if (frame().catcher) {
    push(frame().catcher);
    popc();
    call(1);
  }
  else {
    throw vm_error{except_val};
  }
}

vm::call_frame& vm::machine::frame()
{
  return m_call_stack.back();
}
