#include "expression.h"

#include "opt.h"
#include "vm/instruction.h"

std::vector<vv::vm::command> vv::ast::expression::code() const
{
  auto vec = generate();
  optimize(vec);
  return vec;
}
