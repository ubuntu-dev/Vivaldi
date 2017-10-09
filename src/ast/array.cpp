#include "array.h"

#include "value.h"
#include "vm/instruction.h"

using namespace vv;

ast::array::array(std::vector<std::unique_ptr<ast::expression>>&& members)
  : m_members {move(members)}
{ }

std::vector<vm::command> ast::array::generate() const
{
  std::vector<vm::command> vec;

  for (const auto& i : m_members) {
    const auto arg = i->code();
    copy(begin(arg), end(arg), back_inserter(vec));
  }

  vec.emplace_back(vm::instruction::parr, static_cast<value::integer>(m_members.size()));
  return vec;
}
