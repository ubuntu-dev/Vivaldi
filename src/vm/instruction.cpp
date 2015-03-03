#include "instruction.h"

using namespace vv;

vm::argument::argument()
  : m_val   {0},
    m_which {arg_type::nil}
{ }

vm::argument::argument(int num)
  : m_val   {num},
    m_which {arg_type::num}
{ }

vm::argument::argument(symbol sym)
  : m_val   {sym},
    m_which {arg_type::sym}
{ }

vm::argument::argument(bool bol)
  : m_val   {bol},
    m_which {arg_type::bol}
{ }

vm::argument::argument(const std::string& str)
  : m_val   {str},
    m_which {arg_type::str}
{ }

vm::argument::argument(double flt)
  : m_val   {flt},
    m_which {arg_type::flt}
{ }

vm::argument::argument(const function_t& fnc)
  : m_val   {fnc},
    m_which {arg_type::fnc}
{ }

vm::argument::argument(const type_t& typ)
  : m_val   {typ},
    m_which {arg_type::typ}
{ }

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

int vm::argument::as_int() const
{
  return m_val.num;
}

symbol vm::argument::as_sym() const
{
  return m_val.sym;
}

bool vm::argument::as_bool() const
{
  return m_val.bol;
}

const std::string& vm::argument::as_str() const
{
  return m_val.str;
}

double vm::argument::as_double() const
{
  return m_val.flt;
}

const vm::function_t& vm::argument::as_fn() const
{
  return m_val.fnc;
}

const vm::type_t& vm::argument::as_type() const
{
  return m_val.typ;
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
