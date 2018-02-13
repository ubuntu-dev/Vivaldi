#include "symbol.h"

#include "gc.h"
#include "gc/alloc.h"

std::unordered_map<std::string_view, vv::gc::managed_ptr> vv::symbol::s_symbol_table{ };

vv::symbol::symbol(const std::string_view str)
  : m_ptr {gc::alloc<value::symbol>(str)}
{ }

vv::symbol::symbol(const gc::managed_ptr ptr)
  : m_ptr {ptr.tag() == tag::symbol ? ptr : throw std::runtime_error{"Attempted to construct symbol from non-string"}}
{ }

void vv::symbol::mark()
{
  for (auto&& i : s_symbol_table)
    gc::mark(i.second);
}

std::string_view vv::to_string(symbol sym)
{
  const auto size = *reinterpret_cast<size_t*>(sym.m_ptr.get());
  const auto char_ptr = reinterpret_cast<const char*>(sym.m_ptr.get()) + sizeof(size_t);
  return {char_ptr, size};
}

size_t std::hash<vv::symbol>::operator()(const vv::symbol& sym) const
{
  const static std::hash<vv::gc::managed_ptr> hasher{};
  return hasher(sym.m_ptr);
}
