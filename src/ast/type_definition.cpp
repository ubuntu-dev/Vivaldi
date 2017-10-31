#include "type_definition.h"

#include "gc.h"
#include "vm/instruction.h"

using namespace vv;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    std::unordered_map<vv::symbol, ast::function_definition>&& methods)

  : m_name    {name},
    m_parent  {parent},
    m_methods {move(methods)}
{ }

std::vector<vm::command> ast::type_definition::generate() const
{
  std::vector<vm::command> vec;
  for (const auto& i : m_methods) {
    // the code returned is guaranteed atm to be a single pfn instruction, so
    // just use it directly instead of copying the vector
    vec.push_back(std::move(i.second.code().front()));
    vec.emplace_back( vm::instruction::psym, i.first );
  }

  vec.emplace_back( vm::instruction::psym, m_parent );
  vec.emplace_back( vm::instruction::psym, m_name );

  vec.emplace_back( vm::instruction::ptype,
                    static_cast<value::integer>(m_methods.size()) );

  return vec;
}
