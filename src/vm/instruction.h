#ifndef VV_VM_INSTRUCTIONS_H
#define VV_VM_INSTRUCTIONS_H

#include "symbol.h"

#include <unordered_map>
#include <vector>

namespace vv {

namespace vm {

struct command;

struct function_t {
  int argc;
  std::vector<command> body;
};

struct type_t {
  symbol name;
  symbol parent;
  std::unordered_map<symbol, function_t> methods;
};

enum class instruction {
  /// pushes the provided Bool literal onto the stack
  pbool,
  /// pushes the provided Float literal onto the stack
  pflt,
  /// pushes the provided Function literal onto the stack
  pfn,
  /// pushes the provided Integer literal onto the stack
  pint,
  /// pushes a Nil literal onto the stack
  pnil,
  /// pushes the provided String literal onto the stack
  pstr,
  /// pushes the provided Symbol literal onto the stack
  psym,
  /// Pushes the provided Type literal onto the stack
  ptype,

  /// pushes an Array made out of the provided number of pushed args
  parr,
  /// pushes a Dictionary made out of the provided number of pushed args
  pdict,

  /// reads a variable onto the stack
  read,
  /// writes the top value to a variable
  write,
  /// creates a new variable with the value on top of the stack
  let,

  /// reads self onto the stack
  self,
  /// retrieves the nth passed argument, where n is the provided integer
  arg,
  /// reads a member onto the stack
  readm,
  /// pops the top off the stack, and pushes its member with the provided name
  writem,
  /// calls the top of the stack, using the provided number of pushed arguments
  call,
  /// creates new object of the type on top of the stack
  pobj,

  /// pushes a (shallow) copy of the top of the stack onto the stack
  dup,
  /// removes the provided number of objects from the top of the stack
  pop,

  /// enters a new block
  eblk,
  /// leaves current block
  lblk,
  /// returns from a function; if provided argument is true, copy members of
  /// local frame to parent
  ret,

  /// loads and run a file with the provided name
  req,

  /// unconditionally jumps the provided number of commands
  jmp,
  /// jump the provided number of commands if the top value is falsy
  jf,
  /// jump the provided number of commands if the top value is trutyh
  jt,
  /// pushes top value as a new function for catching exceptions
  pushc,
  /// pops an exception catcher and discards it, leaving top value unchanged
  popc,
  /// throws top value as an exception
  exc,

  /// Changes current directory to that passed as a string literal
  chdir,

  opt_add,
  opt_sub,
  opt_mul,
  opt_div,

  opt_not,
};

// each type in this union is repeated in *quintuplicate*:
// - ctor
// - getter
// - union member
// - union ctor
// - enum
// This is somewhat displeasing, but boost::variant seems to be a slight
// bottleneck, so here we are.
class argument {
public:
  argument()                       : m_val{0},   m_which{arg_type::nil} { }
  argument(int num)                : m_val{num}, m_which{arg_type::num} { }
  argument(symbol sym)             : m_val{sym}, m_which{arg_type::sym} { }
  argument(bool bol)               : m_val{bol}, m_which{arg_type::bol} { }
  argument(const std::string& str) : m_val{str}, m_which{arg_type::str} { }
  argument(double flt)             : m_val{flt}, m_which{arg_type::flt} { }
  argument(const function_t& fnc)  : m_val{fnc}, m_which{arg_type::fnc} { }
  argument(const type_t& typ)      : m_val{typ}, m_which{arg_type::typ} { }

  argument(const argument& other);
  argument(argument&& other);

  argument& operator=(const argument& other);
  argument& operator=(argument&& other);

  int                as_int()    const { return m_val.num; }
  symbol             as_sym()    const { return m_val.sym; }
  bool               as_bool()   const { return m_val.bol; }
  const std::string& as_str()    const { return m_val.str; }
  double             as_double() const { return m_val.flt; }
  const function_t&  as_fn()     const { return m_val.fnc; }
  const type_t&      as_type()   const { return m_val.typ; }

  ~argument();

private:
  union arg_val {
    int         num;
    symbol      sym;
    bool        bol;
    std::string str;
    double      flt;
    function_t  fnc;
    type_t      typ;

    arg_val(int num)                : num{num} { }
    arg_val(symbol sym)             : sym{sym} { }
    arg_val(bool bol)               : bol{bol} { }
    arg_val(const std::string& str) : str{str} { }
    arg_val(double flt)             : flt{flt} { }
    arg_val(const function_t& fnc)  : fnc(fnc) { }
    arg_val(const type_t& typ)      : typ(typ) { }

    ~arg_val() { }
  } m_val;

  enum class arg_type {
    nil,
    num,
    sym,
    bol,
    str,
    flt,
    fnc,
    typ
  } m_which;
};

struct command {
public:
  command(instruction instr, int arg);
  command(instruction instr, symbol arg);
  command(instruction instr, bool arg);
  command(instruction instr, const std::string& arg);
  command(instruction instr, double arg);
  command(instruction instr, const function_t& arg);
  command(instruction instr, const type_t& arg);
  command(instruction instr);

  instruction instr;
  argument arg;
};

}

}

#endif
