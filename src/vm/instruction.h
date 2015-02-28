#ifndef VV_VM_INSTRUCTIONS_H
#define VV_VM_INSTRUCTIONS_H

#include "symbol.h"

#include <boost/variant/variant.hpp>

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
  /// pushes the provided Bool literal into retval
  pbool,
  /// pushes the provided Float literal into retval
  pflt,
  /// pushes the provided Function literal into retval
  pfn,
  /// pushes the provided Integer literal into retval
  pint,
  /// pushes a Nil literal into retval
  pnil,
  /// pushes the provided String literal into retval
  pstr,
  /// pushes the provided Symbol literal into retval
  psym,
  /// Pushes the provided Type literal into retval
  ptype,

  /// sets retval to an Array made out of the provided number of pushed args
  parr,
  /// sets retval to a Dictionary made out of the provided number of pushed args
  pdict,

  /// reads a variable into retval
  read,
  /// writes retval to a variable
  write,
  /// creates a new variable with value retval
  let,

  /// reads self into retval
  self,
  /// retrieves the nth passed argument, where n is the provided integer
  arg,
  /// reads a member into retval
  readm,
  /// sets a member to retval
  writem,
  /// calls retval, using the provided number of pushed arguments
  call,
  /// creates new object of the type in retval
  pobj,

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
  /// jump the provided number of commands if retval is falsy
  jf,
  /// jump the provided number of commands if retval is trutyh
  jt,
  /// pushes retval as a new function for catching exceptions
  pushc,
  /// pops an exception catcher and discards it, leaving retval unchanged
  popc,
  /// throws retval as an exception
  exc,

  /// Changes current directory to that passed as a string literal
  chdir
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
  boost::variant<boost::blank,
                 int,
                 symbol,
                 bool,
                 std::string,
                 double,
                 function_t,
                 type_t>
    arg;
};

}

}

#endif
