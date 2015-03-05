#include "cond_statement.h"

#include "value/nil.h"
#include "vm/instruction.h"

using namespace vv;

ast::cond_statement::cond_statement(
    std::vector<std::pair<std::unique_ptr<expression>,
                std::unique_ptr<expression>>>&& body)
  : m_body {move(body)}
{ }

std::vector<vm::command> ast::cond_statement::generate() const
{
  std::vector<vm::command> vec;
  std::vector<size_t> jump_to_end_idxs;

  vec.emplace_back(vm::instruction::pnil);

  for (const auto& i : m_body) {
    vec.emplace_back(vm::instruction::pop, 1); // pop prev failed test result
    auto test = i.first->code();
    copy(begin(test), end(test), back_inserter(vec));
    vec.emplace_back(vm::instruction::jf);
    auto jump_to_next_test_idx = vec.size() - 1;

    vec.emplace_back(vm::instruction::pop, 1); // pop test result
    auto body = i.second->code();
    copy(begin(body), end(body), back_inserter(vec));
    vec.emplace_back(vm::instruction::jmp);
    jump_to_end_idxs.push_back(vec.size() - 1);

    auto jump_sz = static_cast<int>(vec.size() - jump_to_next_test_idx - 1);
    vec[jump_to_next_test_idx].arg = jump_sz;
  }

  vec.emplace_back(vm::instruction::pop, 1);
  vec.emplace_back(vm::instruction::pnil);
  for (auto i : jump_to_end_idxs) {
    auto jump_sz = static_cast<int>(vec.size() - i - 1);
    vec[i].arg = jump_sz;
  }

  return vec;
}
