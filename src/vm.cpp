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

#include <iostream>

using namespace vv;

vm::machine::machine(call_frame&& frame,
                     const std::function<void(vm::machine&)>& exception_handler)
  : retval              {nullptr},
    m_call_stack        {frame},
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
    } else if (command.instr == vm::instruction::except) {
      except_until(exit_sz);
    } else {
      run_single_command(command);
    }
  }
}

void vm::machine::mark()
{
  for (auto* i : m_stack)
    i->mark();
  for (auto& i : m_call_stack) {
    if (i.caller && !i.caller->marked())
      i.caller->mark();
    if (i.catcher && !i.catcher->marked())
      i.catcher->mark();
    if (!i.env->marked())
      i.env->mark();
  }
  if (retval && !retval->marked())
    retval->mark();
  if (m_pushed_self && !m_pushed_self->marked())
    m_pushed_self->mark();
}

// Instruction implementations {{{

void vm::machine::push_bool(bool val)
{
  retval = gc::alloc<value::boolean>( val );
}

void vm::machine::push_flt(double val)
{
  retval = gc::alloc<value::floating_point>( val );
}

void vm::machine::push_fn(const function_t& val)
{
  retval = gc::alloc<value::function>(val.argc, val.body, frame().env.get());
}

void vm::machine::push_int(int val)
{
  retval = gc::alloc<value::integer>( val );
}

void vm::machine::push_nil()
{
  retval = gc::alloc<value::nil>( );
}

void vm::machine::push_str(const std::string& val)
{
  retval = gc::alloc<value::string>( val );
}

void vm::machine::push_sym(symbol val)
{
  retval = gc::alloc<value::symbol>( val );
}

void vm::machine::push_type(const type_t& type)
{
  read(type.parent);
  auto parent = retval;
  push();
  std::unordered_map<symbol, value::base*> methods;
  for (const auto& i : type.methods) {
    push_fn(i.second);
    auto method = retval;
    push();
    methods[i.first] = method;
  }
  retval = gc::alloc<value::type>( nullptr, methods, *parent, type.name );

  // Clear pushed arguments without touching retval
  m_stack.erase(end(m_stack) - methods.size() - 1, end(m_stack));
  let(type.name);
}

void vm::machine::make_arr(int size)
{
  std::vector<value::base*> args{end(m_stack) - size, end(m_stack)};
  retval = gc::alloc<value::array>( args );
  m_stack.erase(end(m_stack) - size, end(m_stack));
}

void vm::machine::make_dict(int size)
{
  std::unordered_map<value::base*, value::base*> dict;
  for (auto i = end(m_stack) - size; i != end(m_stack); i += 2) {
    dict[i[0]] = i[1];
  }
  retval = gc::alloc<value::dictionary>( dict );
  m_stack.erase(end(m_stack) - size, end(m_stack));
}

void vm::machine::read(symbol sym)
{
  for (auto env = frame().env.get(); env; env = env->enclosing.get()) {
    if (env->local.count(sym)) {
      retval = env->local.at(sym);
      return;
    }
  }
  push_str("no such variable: " + to_string(sym));
  except();
}

void vm::machine::write(symbol sym)
{
  for (auto env = frame().env; env; env = env->enclosing) {
    if (env->local.count(sym)) {
      env->local.at(sym) = retval;
      return;
    }
  }
  push_str("no such variable: " + to_string(sym));
  except();
}

void vm::machine::let(symbol sym)
{
  frame().env->local[sym] = retval;
}

void vm::machine::self()
{
  if (frame().env->self) {
    retval = frame().env->self.get();
  } else {
    push_str("self does not exist outside of objects");
    except();
  }
  assert(retval != nullptr);
  //if (retval->type != &builtin::type::range) assert(retval->value().size() > 0);
}

void vm::machine::arg(int idx)
{
  retval = m_stack[frame().frame_ptr - static_cast<size_t>(idx) - 1];
  assert(retval != nullptr);
}

void vm::machine::readm(symbol sym)
{
  m_pushed_self = retval;
  if (retval->members.count(sym)) {
    retval = retval->members[sym];
    return;
  }

  auto member = find_method(retval->type, sym);
  if (member) {
    retval = member;
    return;
  }
  push_str("no such member: " + to_string(sym));
  except();
}

void vm::machine::writem(symbol sym)
{
  auto value = m_stack.back();
  m_stack.pop_back();

  retval->members[sym] = value;
  retval = value;
}

// TODO: make suck less.
void vm::machine::call(int argc)
{
  const static std::array<vm::command, 1> com{{ {vm::instruction::ret, false} }};
  if (auto fn = dynamic_cast<value::function*>(retval)) {
    if (argc != fn->argc) {
      push_str("Wrong number of arguments--- expected "
              + std::to_string(fn->argc) + ", got "
              + std::to_string(argc));
      except();
      return;
    };

    auto new_env = gc::alloc<environment>( fn->enclosure, m_pushed_self );
    m_call_stack.emplace_back(fn->body, new_env, static_cast<size_t>(argc), m_stack.size());
    frame().caller = fn;

  } else if (auto fn = dynamic_cast<value::builtin_function*>(retval)) {
    if (argc != fn->argc) {
      push_str("Wrong number of arguments--- expected "
              + std::to_string(fn->argc) + ", got "
              + std::to_string(argc));
      except();
      return;
    };

    auto new_env = gc::alloc<environment>( nullptr, m_pushed_self );
    m_call_stack.emplace_back(com, new_env, static_cast<size_t>(argc), m_stack.size());
    frame().caller = fn;

    try {
      retval = fn->body(*this);
    } catch (const vm_error& err) {
      retval = err.error();
      except();
      return;
    }

  } else {
    push_str("Only functions can be called");
    except();
  }
}

void vm::machine::new_obj(int argc)
{
  if (retval->type != &builtin::type::custom_type) {
    push_str("Objects can only be constructed from Types");
    except();
    return;
  }
  auto type = static_cast<value::type*>(retval);

  // Since everything inherits from Object, we're guaranteed to find a
  // constructor
  auto ctr_type = type;
  while (!ctr_type->constructor)
    ctr_type = &static_cast<value::type&>(ctr_type->parent);
  retval = ctr_type->constructor();
  // Hacky--- builtins that can't be instantiated directly have provided
  // constructors that return nullptr, hence this message. The alternative would
  // be them trying to except within their constructors, and tying that behavior
  // back in with the VM can get pretty hairy (as in vm::call)
  if (!retval) {
    push_str("Cannot construct object of type " + type->value());
    except();
    return;
  }
  retval->type = type;

  m_pushed_self = retval;
  push_fn(type->init_shim);
  call(argc);
}

void vm::machine::eblk()
{
  frame().env = gc::alloc<environment>(frame().env.get());
}

void vm::machine::lblk()
{
  frame().env = frame().env->enclosing;
}

void vm::machine::ret(bool copy)
{
  if (m_call_stack.size() > 1) {
    m_stack.erase(begin(m_stack) + frame().frame_ptr - frame().argc, end(m_stack));
    auto& parent = *(end(m_call_stack) - 2);
    if (copy) {
      for (const auto& i : frame().env->local)
        parent.env->local[i.first] = i.second;
    }
    m_call_stack.pop_back();
  } else {
    push_str("The top-level environment can't be returned from");
    except();
  }
}

void vm::machine::push()
{
  m_stack.push_back(retval);
}

void vm::machine::pop()
{
  retval = m_stack.back();
  m_stack.pop_back();
}

void vm::machine::req(const std::string& filename)
{
  auto tok_res = get_file_contents(filename);
  if (!tok_res.successful()) {
    push_str(tok_res.error());
    except();
    return;
  }
  auto cur = boost::filesystem::current_path();
  tok_res.result().emplace_back(instruction::chdir, cur.native());
  tok_res.result().emplace_back(instruction::ret, true);
  push_fn({ 0, move(tok_res.result()) });
  call(0);
}

void vm::machine::jmp(int offset)
{
  frame().instr_ptr = frame().instr_ptr.shifted_by(offset - 1);
}

void vm::machine::jmp_false(int offset)
{
  if (!truthy(retval))
    jmp(offset);
}

void vm::machine::jmp_true(int offset)
{
  if (truthy(retval))
    jmp(offset);
}

void vm::machine::push_catch()
{
  frame().catcher = retval;
}

void vm::machine::pop_catch()
{
  frame().catcher = {};
}

void vm::machine::except()
{
  auto new_frame = find_if(rbegin(m_call_stack), rend(m_call_stack) - 1,
                      [](const auto& i) { return i.catcher; });
  m_call_stack.erase(new_frame.base(), end(m_call_stack));
  m_stack.erase(begin(m_stack) + frame().frame_ptr - frame().argc, end(m_stack));

  if (!frame().catcher) {
    m_exception_handler(*this);
    // If we're still here, stop executing code since obviously some invariant's
    // broken
    frame().instr_ptr = {};//frame().instr_ptr.subvec(frame().instr_ptr.size());
  } else {
    push();
    retval = frame().catcher.get();
    pop_catch();
    call(1);
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
  //   let add = i.add // pushed_self is now i
  //   add(2)          // => 3
  //   5 + 1           // pushed self is now 5
  //   add(2)          // => 7
  if (instr != instruction::call)
    m_pushed_self = nullptr;

  switch (instr) {
  case instruction::push_bool: push_bool(get<bool>(arg));       break;
  case instruction::push_flt:  push_flt(get<double>(arg));      break;
  case instruction::push_fn:   push_fn(get<function_t>(arg));   break;
  case instruction::push_int:  push_int(get<int>(arg));         break;
  case instruction::push_nil:  push_nil();                      break;
  case instruction::push_str:  push_str(get<std::string>(arg)); break;
  case instruction::push_sym:  push_sym(get<symbol>(arg));      break;
  case instruction::push_type: push_type(get<type_t>(arg));     break;

  case instruction::make_arr:  make_arr(get<int>(arg));  break;
  case instruction::make_dict: make_dict(get<int>(arg)); break;

  case instruction::read:  read(get<symbol>(arg));  break;
  case instruction::write: write(get<symbol>(arg)); break;
  case instruction::let:   let(get<symbol>(arg));   break;

  case instruction::self:     self();                   break;
  case instruction::arg:      this->arg(get<int>(arg)); break;
  case instruction::readm:    readm(get<symbol>(arg));  break;
  case instruction::writem:   writem(get<symbol>(arg)); break;
  case instruction::call:     call(get<int>(arg));      break;
  case instruction::new_obj:  new_obj(get<int>(arg));   break;

  case instruction::eblk: eblk();              break;
  case instruction::lblk: lblk();              break;
  case instruction::ret:  ret(get<bool>(arg)); break;

  case instruction::push: push(); break;
  case instruction::pop:  pop();  break;

  case instruction::req: req(get<std::string>(arg)); break;

  case instruction::jmp:       jmp(get<int>(arg));       break;
  case instruction::jmp_false: jmp_false(get<int>(arg)); break;
  case instruction::jmp_true:  jmp_true(get<int>(arg));  break;

  case instruction::push_catch: push_catch(); break;
  case instruction::pop_catch:  pop_catch();  break;
  case instruction::except:     except();     break;

  case instruction::chdir:      chdir(get<std::string>(arg)); break;
  }
}

void vm::machine::except_until(size_t stack_pos)
{
  auto last = find_if(rbegin(m_call_stack), rend(m_call_stack) - stack_pos,
                      [](const auto& i) { return i.catcher; });
  auto new_frame = std::max(last.base(), begin(m_call_stack) + stack_pos);
  m_call_stack.erase(new_frame, end(m_call_stack));
  m_stack.erase(begin(m_stack) + frame().frame_ptr - frame().argc, end(m_stack));

  if (!frame().catcher) {
    throw vm_error{retval};

  } else {
    push();
    retval = frame().catcher.get();
    pop_catch();
    call(1);
  }
}

vm::call_frame& vm::machine::frame()
{
  return m_call_stack.back();
}
