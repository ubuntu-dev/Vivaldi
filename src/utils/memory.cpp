#include "memory.h"

#include <cassert>

using namespace vv;

free_block_list::iterator free_block_list::insert(value_type ptr)
{
  auto next = m_data.lower_bound(ptr);
  if (next != end() && next->first == ptr.first + ptr.second) {
    ptr.second += next->second;
    next = m_data.erase(next);
  }

  if (next != begin()) {
    auto prev = --iterator{next};
    if (prev->first + prev->second == ptr.first) {
      ptr.first = prev->first;
      ptr.second += prev->second;
      m_data.erase(prev);
    }
  }

  return m_data.insert(next, ptr);
}
