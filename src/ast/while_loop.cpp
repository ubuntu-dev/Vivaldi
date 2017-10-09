#include "while_loop.h"

#include "value.h"
#include "vm/instruction.h"

using namespace vv;

ast::while_loop::while_loop(std::unique_ptr<expression>&& test,
                            std::unique_ptr<expression>&& body)
  : m_test {move(test)},
    m_body {move(body)}
{ }

std::vector<vm::command> ast::while_loop::generate() const
{
  auto vec = m_test->code();

  const auto test_idx = vec.size();
  vec.emplace_back(vm::instruction::jf);
  vec.emplace_back(vm::instruction::pop, 1); //test result

  const auto body = m_body->code();
  for (auto&& i : body)
    vec.push_back(std::move(i));

  vec.emplace_back(vm::instruction::pop, 1); // expr result
  vec.emplace_back(vm::instruction::jmp, -static_cast<value::integer>(vec.size() + 1));

  vec[test_idx].arg = static_cast<value::integer>(vec.size() - test_idx) - 1;
  vec.emplace_back(vm::instruction::pop, 1); // failed test
  vec.emplace_back(vm::instruction::pnil);

  return vec;
}
