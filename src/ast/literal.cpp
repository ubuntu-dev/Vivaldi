#include "literal.h"

#include "vm/instruction.h"

using namespace vv;

std::vector<vm::command> ast::literal::boolean::generate() const
{
  return { {vm::instruction::pbool, m_val} };
}

std::vector<vm::command> ast::literal::floating_point::generate() const
{
  return { {vm::instruction::pflt, m_val} };
}

std::vector<vm::command> ast::literal::integer::generate() const
{
  return { {vm::instruction::pint, m_val} };
}

std::vector<vm::command> ast::literal::nil::generate() const
{
  return { {vm::instruction::pnil} };
}

std::vector<vm::command> ast::literal::regex::generate() const
{
  return { {vm::instruction::pre, m_val} };
}

std::vector<vm::command> ast::literal::string::generate() const
{
  return { {vm::instruction::pstr, m_val} };
}

std::vector<vm::command> ast::literal::symbol::generate() const
{
  return { {vm::instruction::psym, m_val} };
}
