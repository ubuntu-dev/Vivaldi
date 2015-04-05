#include "member.h"

#include "vm/instruction.h"

using namespace vv;

ast::member::member(vv::symbol name)
  : m_name   {name}
{ }

std::vector<vm::command> ast::member::generate() const
{
  return { { vm::instruction::readm, m_name } };
}
