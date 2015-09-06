#include "function_definition.h"

#include "opt.h"
#include "value/function.h"

using namespace vv;

ast::function_definition::function_definition(symbol name,
                                              std::unique_ptr<expression>&&body,
                                              const std::vector<symbol>& args,
                                              boost::optional<symbol> vararg_name)
  : m_name        {name},
    m_body        {move(body)},
    m_args        {args},
    m_vararg_name {vararg_name}
{ }

std::vector<vm::command> ast::function_definition::generate() const
{
  const auto argc = static_cast<int>(m_args.size());
  std::vector<vm::command> definition;
  for (auto i = argc; i--;) {
    definition.emplace_back(vm::instruction::arg, i);
    definition.emplace_back(vm::instruction::let, m_args[i]);
    definition.emplace_back(vm::instruction::pop, 1);
  }

  if (m_vararg_name) {
    definition.emplace_back(vm::instruction::varg, argc);
    definition.emplace_back(vm::instruction::let, *m_vararg_name);
    definition.emplace_back(vm::instruction::pop, 1);
  }

  const auto body = m_body->code();
  copy(begin(body), end(body), back_inserter(definition));
  definition.emplace_back(vm::instruction::ret, false);

  optimize_independent_block(definition);

  std::vector<vm::command> vec;
  // ternary == poor man's cast cause I can't be bothered to look at the
  // boost::optional docs atm
  vec.emplace_back( vm::instruction::pfn,
                    vm::function_t{argc, move(definition), m_vararg_name ? true : false} );

  if (m_name != symbol{})
    vec.emplace_back(vm::instruction::let, m_name);

  return vec;
}
