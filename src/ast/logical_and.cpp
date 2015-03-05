#include "logical_and.h"

#include "vm/instruction.h"

using namespace vv;

ast::logical_and::logical_and(std::unique_ptr<expression>&& left,
                              std::unique_ptr<expression>&& right)
  : m_left  {move(left)},
    m_right {move(right)}
{ }

std::vector<vm::command> ast::logical_and::generate() const
{
  // Given conditions 'a' and 'b', generate the following VM instructions:
  //   a
  //   jmp_false <false index - this index>
  //   b
  //   jmp_false 3
  //   push_bool true
  //   jmp 2
  //   push_bool false
  auto vec = m_left->code();
  vec.emplace_back(vm::instruction::jf);
  auto jmp_to_false_idx = vec.size() - 1;
  vec.emplace_back(vm::instruction::pop, 1);

  auto right = m_right->code();
  copy(begin(right), end(right), back_inserter(vec));
  vec.emplace_back(vm::instruction::jf, 3);
  vec.emplace_back(vm::instruction::pop, 1);
  vec.emplace_back(vm::instruction::pbool, true);
  vec.emplace_back(vm::instruction::jmp, 2);
  vec.emplace_back(vm::instruction::pop, 1);
  vec.emplace_back(vm::instruction::pbool, false);

  auto false_idx = vec.size() - 1;
  vec[jmp_to_false_idx].arg = static_cast<int>(false_idx - jmp_to_false_idx);
  return vec;
}
