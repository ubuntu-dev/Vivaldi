#include "logical_or.h"

#include "value.h"
#include "vm/instruction.h"

using namespace vv;

ast::logical_or::logical_or(std::unique_ptr<expression>&& left,
                              std::unique_ptr<expression>&& right)
  : m_left  {move(left)},
    m_right {move(right)}
{ }

std::vector<vm::command> ast::logical_or::generate() const
{
  // Given conditions 'a' and 'b', generate the following VM instructions:
  //   a
  //   jmp_true <true index - this index>
  //   b
  //   jmp_true 3
  //   push_bool false
  //   jmp 2
  //   push_bool true
  auto vec = m_left->code();
  vec.emplace_back(vm::instruction::jt);
  const auto jmp_to_false_idx = vec.size() - 1;
  vec.emplace_back(vm::instruction::pop, 1);

  const auto right = m_right->code();
  copy(begin(right), end(right), back_inserter(vec));
  vec.emplace_back(vm::instruction::jt, 3);
  vec.emplace_back(vm::instruction::pop, 1);
  vec.emplace_back(vm::instruction::pbool, false);
  vec.emplace_back(vm::instruction::jmp, 2);
  vec.emplace_back(vm::instruction::pop, 1);
  vec.emplace_back(vm::instruction::pbool, true);

  const auto false_idx = vec.size() - 1;
  vec[jmp_to_false_idx].arg = static_cast<value::integer>(false_idx - jmp_to_false_idx);
  return vec;
}
