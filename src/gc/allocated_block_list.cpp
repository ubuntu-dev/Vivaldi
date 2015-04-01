#include "allocated_block_list.h"

using namespace vv;
using namespace gc;

bool allocated_block_list::contains(void* ptr)
{
  auto iter = m_blocks.upper_bound(ptr);
  if (iter == begin(m_blocks))
    return false;

  --iter;
  if (static_cast<char*>(iter->first) + 65'536 <= static_cast<char*>(ptr))
    return false;
  return true;
}

bool allocated_block_list::marked(void* ptr)
{
  auto iter = --m_blocks.upper_bound(ptr);
  auto dist = static_cast<char*>(ptr) - static_cast<char*>(iter->first);
  return iter->second[dist / 8];
}

void allocated_block_list::mark(void* ptr)
{
  auto iter = --m_blocks.upper_bound(ptr);
  auto dist = static_cast<char*>(ptr) - static_cast<char*>(iter->first);
  iter->second.set(dist / 8);
}

void allocated_block_list::unmark()
{
  for (auto& i : m_blocks)
    i.second.reset();
}

void allocated_block_list::insert_block(void* block_start)
{
  m_blocks[block_start] = {};
}
