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
