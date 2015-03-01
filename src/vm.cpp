#include "vm.h"

#include "builtins.h"
#include "gc.h"
#include "get_file_contents.h"
#include "parser.h"
#include "value.h"
#include "utils/error.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/boolean.h"
#include "value/dictionary.h"
#include "value/integer.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/nil.h"
#include "value/string.h"
#include "value/symbol.h"

#include <boost/variant/get.hpp>
#include <boost/filesystem.hpp>

using namespace vv;

vm::machine::machine(call_frame&& frame,
                     const std::function<void(vm::machine&)>& exception_handler)
  : m_call_stack        {frame},
    m_exception_handler {exception_handler}
{
  gc::set_running_vm(*this);
}

void vm::machine::run()
{
  while (frame().instr_ptr.size()) {
    // Get next instruction (and argument, if it exists), and increment the
    // instruction pointer
    auto command = frame().instr_ptr.front();
    frame().instr_ptr = frame().instr_ptr.subvec(1);
    run_single_command(command);
  }
}

void vm::machine::run_cur_scope()
{
  auto exit_sz = m_call_stack.size();

  while (frame().instr_ptr.size()) {
    // Get next instruction (and argument, if it exists), and increment the
    // instruction pointer
    auto command = frame().instr_ptr.front();
    frame().instr_ptr = frame().instr_ptr.subvec(1);

    if (command.instr == vm::instruction::ret && m_call_stack.size() == exit_sz) {
      ret(boost::get<bool>(command.arg));
      return;
    } else if (command.instr == vm::instruction::exc) {
      except_until(exit_sz);
    } else {
      run_single_command(command);
    }
  }
}

value::base* vm::machine::top()
{
  return m_stack.back();
}

void vm::machine::push(value::base* val)
{
  m_stack.push_back(val);
}

void vm::machine::mark()
{
  for (auto* i : m_stack)
    if (!i->marked())
      i->mark();

  if (m_transient_self && !m_transient_self->marked())
    m_transient_self->mark();

  for (auto& i : m_call_stack) {
    if (i.caller && !i.caller->marked())
      i.caller->mark();
    if (i.catcher && !i.catcher->marked())
      i.catcher->mark();
    if (!i.env->marked())
      i.env->mark();
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
  push(gc::alloc<value::function>(val.argc, val.body, frame().env.get()));
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
  auto parent = top();

  std::unordered_map<symbol, value::base*> methods;
  for (const auto& i : type.methods) {
    pfn(i.second);
    methods[i.first] = top();
  }
  auto newtype = gc::alloc<value::type>( nullptr, methods, *parent, type.name );

  pop(methods.size() + 1); // methods and parent
  push(newtype);
  let(type.name);
}

void vm::machine::parr(int size)
{
  std::vector<value::base*> vec{end(m_stack) - size, end(m_stack)};
  auto val = gc::alloc<value::array>( move(vec) );
  pop(size);
  push(val);
}

void vm::machine::pdict(int size)
{
  std::unordered_map<value::base*, value::base*> dict;
  for (auto i = end(m_stack) - size; i != end(m_stack); i += 2)
    dict[i[0]] = i[1];

  auto val = gc::alloc<value::dictionary>( dict );
  pop(size);
  push(val);
}

void vm::machine::read(symbol sym)
{
  for (auto i = frame().env; i; i = i->enclosing) {
    if (i->local.count(sym)) {
      push(i->local.at(sym));
      return;
    }
  }
  pstr("no such variable: " + to_string(sym));
  exc();
}

void vm::machine::write(symbol sym)
{
  for (auto i = frame().env; i; i = i->enclosing) {
    if (i->local.count(sym)) {
      i->local.at(sym) = top();
      return;
    }
  }
  pstr("no such variable: " + to_string(sym));
  exc();
}

void vm::machine::let(symbol sym)
{
  if (frame().env->local.count(sym)) {
    pstr("variable " + to_string(sym) += " already exists");
    exc();
  } else {
    frame().env->local[sym] = top();
  }
}

void vm::machine::self()
{
  if (frame().env->self) {
    push(frame().env->self.get());
  } else {
    pstr("self cannot be accessed outside of objects");
    exc();
  }
}

void vm::machine::arg(int idx)
{
  push(*(begin(m_stack) + frame().frame_ptr - idx));
}

void vm::machine::readm(symbol sym)
{
  m_transient_self = top();
  pop(1);

  if (m_transient_self->members.count(sym)) {
    push(m_transient_self->members.at(sym));
  } else if (auto method = find_method(m_transient_self->type, sym)) {
    push(method);
  } else {
    pstr(m_transient_self->value() += " has no member " + to_string(sym));
    exc();
  }
}

void vm::machine::writem(symbol sym)
{
  auto obj = top();
  m_stack.pop_back();
  obj->members[sym] = top();
}

// TODO: factor out argc, so I can just do this check once in machine::call
namespace {

void exc_wrong_argc(vm::machine& vm, int expected, int recieved)
{
  vm.pstr("wrong number of arguments--- expected " + std::to_string(expected)
       += ", got " + std::to_string(recieved));
  vm.exc();
}

};

void vm::machine::call(int argc)
{
  const static std::array<command, 1> builtin_shim {{
    { instruction::ret, false }
  }};
  auto func = top();
  pop(1);

  if (auto builtin = dynamic_cast<value::builtin_function*>(func)) {
    if (builtin->argc != argc) {
      exc_wrong_argc(*this, builtin->argc, argc);
      return;
    }

    auto new_env = gc::alloc<environment>( nullptr, m_transient_self );
    m_call_stack.emplace_back(builtin_shim, new_env, argc, m_stack.size() - 1);
    frame().caller = func;
    try {
      push(builtin->body(*this));
    } catch (const vm_error& h) {
      push(h.error());
      exc();
      return;
    };

  } else if (auto vvfunc = dynamic_cast<value::function*>(func)) {
    if (vvfunc->argc != argc) {
      exc_wrong_argc(*this, builtin->argc, argc);
      return;
    }
    auto new_env = gc::alloc<environment>(vvfunc->enclosure, m_transient_self);
    m_call_stack.emplace_back(vvfunc->body, new_env, argc, m_stack.size() - 1);
    frame().caller = func;

  } else {
    m_call_stack.pop_back();
    pstr("objects of type " + func->type->value() += " cannot be called");
    exc();
  }
}

void vm::machine::pobj(int argc)
{
  if (top()->type != &builtin::type::custom_type) {
    pstr("objects can only be constructed from Types");
    exc();
    return;
  }

  auto type = static_cast<value::type*>(top());

  auto ctor_type = type;
  while (!ctor_type->constructor)
    ctor_type = static_cast<value::type*>(&ctor_type->parent);
  pop(1);
  push(ctor_type->constructor());
  // Hack--- nonconstructible types (e.g. Integer) have constructors that return
  // nullptr
  if (!top()) {
    pop(1);
    pstr("cannot directly construct objects of type " + ctor_type->value());
    return;
  }
  top()->type = type;
  m_transient_self = top();
  pop(1);
  pfn(type->init_shim);
  call(argc);
}

void vm::machine::dup()
{
  push(top());
}

void vm::machine::pop(int quant)
{
  m_stack.erase(end(m_stack) - quant, end(m_stack));
}

void vm::machine::eblk()
{
  frame().env = gc::alloc<environment>( frame().env.get() );
}

void vm::machine::lblk()
{
  frame().env = frame().env->enclosing;
}

void vm::machine::ret(bool copy)
{
  auto retval = top();
  m_stack.erase(begin(m_stack) + frame().frame_ptr - frame().argc + 1, end(m_stack));
  //pop(frame().frame_ptr + frame().argc);
  push(retval);

  auto cur_env = frame().env;
  m_call_stack.pop_back();
  if (copy)
    for (const auto& i : cur_env->local)
      frame().env->local[i.first] = i.second;
}

void vm::machine::req(const std::string& filename)
{
  auto contents = get_file_contents(filename);
  if (!contents.successful()) {
    pstr(contents.error());
    exc();
    return;
  }
  contents.result().emplace_back(instruction::ret, true);
  pfn(function_t{0, contents.result()});
  call(0);
}

void vm::machine::jmp(int offset)
{
  frame().instr_ptr = frame().instr_ptr.shifted_by(offset);
}

void vm::machine::jf(int offset)
{
  if (!truthy(top()))
    jmp(offset);
}

void vm::machine::jt(int offset)
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
  frame().catcher = nullptr;
}

void vm::machine::exc()
{
  auto except_val = top();

  auto last = find_if(rbegin(m_call_stack), rend(m_call_stack) - 1,
                      [](const auto& i) { return i.catcher; });

  if (last != rbegin(m_call_stack)) {
    auto last_erased = last.base();
    m_stack.erase(begin(m_stack) + last_erased->frame_ptr - last_erased->argc + 1,
                  end(m_stack));
  }
  m_call_stack.erase(last.base(), end(m_call_stack));
  push(except_val);

  if (frame().catcher) {
    push(frame().catcher.get());
    popc();
    call(1);
  } else {
    m_exception_handler(*this);
    frame().instr_ptr = {}; // bail out before anything blows up
  }
}

void vm::machine::chdir(const std::string& dir)
{
  boost::filesystem::current_path(dir);
}

// }}}

void vm::machine::run_single_command(const vm::command& command)
{
  using boost::get;

  auto instr = command.instr;
  const auto& arg = command.arg;

  // HACK--- avoid weirdness like the following:
  //   let i = 1
  //   let add = i.add // self is now i
  //   add(2)          // => 3
  //   5 + 1           // self is now 5
  //   add(2)          // => 7
  if (instr != instruction::call)
    m_transient_self = nullptr;

  switch (instr) {
  case instruction::pbool: pbool(get<bool>(arg));       break;
  case instruction::pflt:  pflt(get<double>(arg));      break;
  case instruction::pfn:   pfn(get<function_t>(arg));   break;
  case instruction::pint:  pint(get<int>(arg));         break;
  case instruction::pnil:  pnil();                      break;
  case instruction::pstr:  pstr(get<std::string>(arg)); break;
  case instruction::psym:  psym(get<symbol>(arg));      break;
  case instruction::ptype: ptype(get<type_t>(arg));     break;

  case instruction::parr:  parr(get<int>(arg));  break;
  case instruction::pdict: pdict(get<int>(arg)); break;

  case instruction::read:  read(get<symbol>(arg));  break;
  case instruction::write: write(get<symbol>(arg)); break;
  case instruction::let:   let(get<symbol>(arg));   break;

  case instruction::self:   self();                   break;
  case instruction::arg:    this->arg(get<int>(arg)); break;
  case instruction::readm:  readm(get<symbol>(arg));  break;
  case instruction::writem: writem(get<symbol>(arg)); break;
  case instruction::call:   call(get<int>(arg));      break;
  case instruction::pobj:   pobj(get<int>(arg));      break;

  case instruction::eblk: eblk();              break;
  case instruction::lblk: lblk();              break;
  case instruction::ret:  ret(get<bool>(arg)); break;

  case instruction::dup: dup();              break;
  case instruction::pop: pop(get<int>(arg)); break;

  case instruction::req: req(get<std::string>(arg)); break;

  case instruction::jmp: jmp(get<int>(arg)); break;
  case instruction::jf:  jf(get<int>(arg));  break;
  case instruction::jt:  jt(get<int>(arg));  break;

  case instruction::pushc: pushc(); break;
  case instruction::popc:  popc();  break;
  case instruction::exc:   exc();   break;

  case instruction::chdir:      chdir(get<std::string>(arg)); break;
  }
}

void vm::machine::except_until(size_t stack_pos)
{
  auto except_val = top();

  auto last = find_if(rbegin(m_call_stack), rend(m_call_stack) - stack_pos,
                      [](const auto& i) { return i.catcher; });

  auto last_erased = last.base();
  m_stack.erase(begin(m_stack) + last_erased->frame_ptr - last_erased->argc,
                end(m_stack));
  m_call_stack.erase(last.base(), end(m_call_stack));
  push(except_val);


  if (frame().catcher) {
    push(frame().catcher.get());
    popc();
    call(1);
  } else {
    throw vm_error{except_val};
  }
}

vm::call_frame& vm::machine::frame()
{
  return m_call_stack.back();
}
