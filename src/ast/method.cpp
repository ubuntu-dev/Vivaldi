#include "method.h"

#include "vm/instruction.h"

using namespace vv;

ast::method::method(std::unique_ptr<ast::expression>&& object, vv::symbol name)
  : m_object {move(object)},
    m_name   {name}
{ }

std::vector<vm::command> ast::method::generate() const
{
  auto vec = m_object->code();
  vec.emplace_back(vm::instruction::method, m_name);
  return vec;
}
