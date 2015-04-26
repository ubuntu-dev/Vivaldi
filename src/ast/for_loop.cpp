#include "for_loop.h"

#include "vm/instruction.h"

using namespace vv;

ast::for_loop::for_loop(symbol iterator,
                        std::unique_ptr<expression>&& range,
                        std::unique_ptr<expression>&& body)
  : m_iterator {iterator},
    m_range    {move(range)},
    m_body     {move(body)}
{ }

// TODO: clean up substantially
std::vector<vm::command> ast::for_loop::generate() const
{
  auto vec = m_range->code();
  vec.emplace_back(vm::instruction::method, symbol{"start"});
  vec.emplace_back(vm::instruction::call, 0);

  const auto test_idx = static_cast<int>(vec.size() - 1);
  vec.emplace_back(vm::instruction::dup);
  vec.emplace_back(vm::instruction::opt_at_end);
  vec.emplace_back(vm::instruction::jt);
  const auto jmp_to_end_idx = static_cast<int>(vec.size() - 1);

  vec.emplace_back(vm::instruction::eblk); // enter new scope for iterator var
  vec.emplace_back(vm::instruction::pop, 1); // clear result of at_end test
  vec.emplace_back(vm::instruction::dup); // duplicate iterator
  vec.emplace_back(vm::instruction::opt_get);
  vec.emplace_back(vm::instruction::let, m_iterator);
  vec.emplace_back(vm::instruction::pop, 1); // clear m_iterator value
  const auto body_code = m_body->code();
  copy(begin(body_code), end(body_code), back_inserter(vec));
  vec.emplace_back(vm::instruction::lblk);

  vec.emplace_back(vm::instruction::pop, 1); // clear result of body code

  vec.emplace_back(vm::instruction::dup); // duplicate iterator
  vec.emplace_back(vm::instruction::opt_incr);
  vec.emplace_back(vm::instruction::pop, 1); // clear garbage returned by incr

  vec.emplace_back(vm::instruction::jmp);
  const auto jmp_back_idx = static_cast<int>(vec.size() - 1);

  const auto end_idx = static_cast<int>(vec.size());
  vec.emplace_back(vm::instruction::pop, 2); // clear iterator and at_end result
  vec.emplace_back(vm::instruction::pnil);

  vec[jmp_to_end_idx].arg = end_idx - jmp_to_end_idx;
  vec[jmp_back_idx].arg = test_idx - jmp_back_idx;

  return vec;
}
