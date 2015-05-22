#include "try_catch.h"

#include "gc.h"

using namespace vv;

ast::try_catch::try_catch(std::unique_ptr<expression>&& body,
                          std::vector<catch_stmt>&& catchers)
  : m_body     {move(body)},
    m_catchers {move(catchers)}
{ }

std::vector<vm::command> ast::try_catch::generate() const
{
  std::vector<vm::command> vec;

  for (const auto& i : m_catchers) {
    std::vector<vm::command> catcher;
    catcher.emplace_back(vm::instruction::arg, 0);

    catcher.emplace_back(vm::instruction::let, i.exception_name);
    const auto catcher_body = i.catcher->code();
    copy(begin(catcher_body), end(catcher_body), back_inserter(catcher));
    catcher.emplace_back(vm::instruction::ret, false);

    vec.emplace_back(vm::instruction::pfn, vm::function_t{1, move(catcher)});
    vec.emplace_back(vm::instruction::pushc, i.exception_type);
  }

  auto body = m_body->code();
  body.emplace_back(vm::instruction::ret, false);
  vec.emplace_back(vm::instruction::pfn, vm::function_t{0, move(body)});
  vec.emplace_back(vm::instruction::call, 0);

  for (const auto& i : m_catchers)
    vec.emplace_back(vm::instruction::popc, i.exception_type);

  return vec;
}
