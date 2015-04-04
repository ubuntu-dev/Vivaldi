#include "managed_ptr.h"

#include "builtins.h"
#include "gc/block_list.h"

using namespace vv;
using namespace gc;

managed_ptr::managed_ptr()
  : m_block  {0},
    m_offset {0},
    m_tag    {tag::nil},
    m_flags  {0}
{ }

value::basic_object* managed_ptr::get() const
{
  const auto char_ptr = internal::g_blocks.m_list[m_block]->block.data() + m_offset;
  return reinterpret_cast<value::basic_object*>(char_ptr);
}

vv::tag managed_ptr::tag() const
{
  return m_tag;
}

managed_ptr managed_ptr::type() const
{
  switch (m_tag) {
  case tag::nil:     return builtin::type::nil;
  case tag::integer: return builtin::type::integer;
  case tag::boolean: return builtin::type::boolean;
  default:           return get()->type;
  }
}

managed_ptr::operator bool() const
{
  return m_flags != 0;
}

bool gc::operator==(managed_ptr lhs, managed_ptr rhs)
{
  return lhs.m_block == rhs.m_block && lhs.m_offset == rhs.m_offset
      && lhs.m_tag == rhs.m_tag     && lhs.m_flags == rhs.m_flags;
}

bool gc::operator!=(managed_ptr lhs, managed_ptr rhs)
{
  return !(lhs == rhs);
}

size_t std::hash<managed_ptr>::operator()(managed_ptr ptr) const
{
  const static std::hash<uint64_t> hasher{};
  return hasher(*reinterpret_cast<uint64_t*>(&ptr));
}