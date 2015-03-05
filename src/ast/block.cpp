#include "block.h"

#include "vm/instruction.h"

using namespace vv;

ast::block::block(std::vector<std::unique_ptr<expression>>&& subexpressions)
  : m_subexpressions {move(subexpressions)}
{ }

std::vector<vm::command> ast::block::generate() const
{
  // Conceptually, *every* block statement consists of
  //   eblk
  //   pnil
  //   pop
  //   expr_1
  //   pop
  //   expr_2
  //   pop
  //   ...
  //   expr_n
  //   lblk
  // But since the only time the pnil is actually used is when there are no
  // expressions, and since in that case the e/lblk don't change any semantics,
  // there's no reason not to special-case it

  std::vector<vm::command> vec{ {vm::instruction::eblk} };
  vec.emplace_back(vm::instruction::pnil);

  for (const auto& i : m_subexpressions) {
    auto subexpr = i->code();
    vec.emplace_back(vm::instruction::pop, 1);
    copy(begin(subexpr), end(subexpr), back_inserter(vec));
  }

  vec.push_back(vm::instruction::lblk);
  return vec;
}
