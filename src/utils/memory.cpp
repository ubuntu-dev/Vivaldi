#include "memory.h"

#include <cassert>

using namespace vv;

free_block_list::iterator free_block_list::insert(value_type ptr)
{
  auto iter = m_data.insert(ptr).first;
  if (iter != begin()) {
    auto prev = iter;
    --prev;
    if (prev->first + prev->second == iter->first) {
      auto new_data = std::make_pair(prev->first, prev->second + iter->second);
      m_data.erase(prev);
      m_data.erase(iter);
      iter = m_data.insert(new_data).first;
    }
  }

  auto next = iter;
  ++next;
  if (next != end() && iter->first + iter->second == next->first) {
    auto new_data = std::make_pair(iter->first, iter->second + next->second);
    m_data.erase(iter);
    m_data.erase(next);
    iter = m_data.insert(new_data).first;
  }
  return iter;
}
