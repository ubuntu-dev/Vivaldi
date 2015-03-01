#include "while_loop.h"

#include "value/nil.h"
#include "vm/instruction.h"

using namespace vv;

ast::while_loop::while_loop(std::unique_ptr<expression>&& test,
                            std::unique_ptr<expression>&& body)
  : m_test {move(test)},
    m_body {move(body)}
{ }

std::vector<vm::command> ast::while_loop::generate() const
{
  auto vec = m_test->generate();

  auto test_idx = vec.size();
  vec.emplace_back(vm::instruction::jf);
  vec.emplace_back(vm::instruction::pop, 1); //test result

  auto body = m_body->generate();
  for (auto&& i : body)
    vec.push_back(std::move(i));

  vec.emplace_back(vm::instruction::pop, 1); // expr result
  vec.emplace_back(vm::instruction::jmp, -static_cast<int>(vec.size() + 1));

  vec[test_idx].arg = static_cast<int>(vec.size() - test_idx);
  vec.emplace_back(vm::instruction::pop, 1); // failed test
  vec.emplace_back(vm::instruction::pnil);

  return vec;
}
