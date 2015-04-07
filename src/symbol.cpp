#include "symbol.h"

std::unordered_set<std::string> vv::symbol::s_symbol_table{ };

vv::symbol::symbol(const std::string& str)
  : m_ptr {&*s_symbol_table.insert(str).first}
{ }

const std::string& vv::to_string(symbol sym)
{
  return *sym.m_ptr;
}

size_t std::hash<vv::symbol>::operator()(const vv::symbol& sym) const
{
  const static std::hash<const std::string*> hasher{};
  return hasher(sym.m_ptr);
}
