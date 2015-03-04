#include "instruction.h"

using namespace vv;

vm::argument::argument(const argument& other)
  : m_val   {0},
    m_which {other.m_which}
{
  // Just assign normally, unless type has nontrivial destructor/copy ctor, in
  // which case doing so would be ill-advised
  switch (m_which) {
  case arg_type::nil:                                            break;
  case arg_type::num: m_val.num = other.m_val.num;               break;
  case arg_type::sym: m_val.sym = other.m_val.sym;               break;
  case arg_type::bol: m_val.bol = other.m_val.bol;               break;
  case arg_type::str: new (&m_val) std::string{other.m_val.str}; break;
  case arg_type::flt: m_val.flt = other.m_val.flt;               break;
  case arg_type::fnc: new (&m_val) function_t (other.m_val.fnc); break;
  case arg_type::typ: new (&m_val) type_t     (other.m_val.typ); break;
  }
}

vm::argument::argument(argument&& other)
  : m_val   {0},
    m_which {other.m_which}
{
  // as per above, this time with std::move for added movement
  switch (m_which) {
  case arg_type::nil:                                                       break;
  case arg_type::num: m_val.num = other.m_val.num;                          break;
  case arg_type::sym: m_val.sym = other.m_val.sym;                          break;
  case arg_type::bol: m_val.bol = other.m_val.bol;                          break;
  case arg_type::str: new (&m_val) std::string{std::move(other.m_val.str)}; break;
  case arg_type::flt: m_val.flt = other.m_val.flt;                          break;
  case arg_type::fnc: new (&m_val) function_t (std::move(other.m_val.fnc)); break;
  case arg_type::typ: new (&m_val) type_t     (std::move(other.m_val.typ)); break;
  }
}

vm::argument& vm::argument::operator=(const argument& other)
{
  auto tmp = other;
  std::swap(*this, tmp);
  return *this;
}

vm::argument& vm::argument::operator=(argument&& other)
{
  this->~argument(); // destruct current m_val
  m_which = other.m_which;
  // reprise
  switch (m_which) {
  case arg_type::nil:                                                       break;
  case arg_type::num: m_val.num = other.m_val.num;                          break;
  case arg_type::sym: m_val.sym = other.m_val.sym;                          break;
  case arg_type::bol: m_val.bol = other.m_val.bol;                          break;
  case arg_type::str: new (&m_val) std::string{std::move(other.m_val.str)}; break;
  case arg_type::flt: m_val.flt = other.m_val.flt;                          break;
  case arg_type::fnc: new (&m_val) function_t (std::move(other.m_val.fnc)); break;
  case arg_type::typ: new (&m_val) type_t     (std::move(other.m_val.typ)); break;
  }
  return *this;
}

vm::argument::~argument()
{
  using std::string;
  switch (m_which) {
  case arg_type::str: m_val.str.~string();     break;
  case arg_type::fnc: m_val.fnc.~function_t(); break;
  case arg_type::typ: m_val.typ.~type_t();     break;
  default: ;
  }
}

vm::command::command(instruction new_instr, int new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr, symbol new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr, bool new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr, const std::string& new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr, double new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr, const function_t& new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr, const type_t& new_arg)
  : instr {new_instr},
    arg   {new_arg}
{ }

vm::command::command(instruction new_instr)
  : instr {new_instr} // arg is default-constructed to boost::blank
{ }
