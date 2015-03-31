#include "free_block_list.h"

using namespace vv;

void* gc::free_block_list::allocate(size_t sz)
{
  auto it = find_if(begin(m_list), end(m_list),
                    [sz](auto blk) { return blk.size >= sz; });

  if (it == end(m_list))
    return nullptr;

  auto ptr = it->ptr;

  if (it->size > sz) {
    it->ptr += sz;
    it->size -= sz;
  }
  else {
    // whoops
    m_list.erase(it);
  }
  return ptr;
}

void gc::free_block_list::reclaim(void* ptr, size_t size)
{
  free_block blk{size, static_cast<char*>(ptr)};
  auto pos = upper_bound(begin(m_list), end(m_list), blk,
                         [](auto a, auto b) { return a.ptr < b.ptr; });

  if (pos != begin(m_list)) {
    auto prev = pos - 1;
    if (prev->ptr + prev->size == blk.ptr) {
      prev->size += blk.size;
      if (pos != end(m_list) && pos->ptr == prev->ptr + prev->size) {
        prev->size += pos->size;
        m_list.erase(pos);
      }
      return;
    }
  }

  if (pos != end(m_list) && pos->ptr == blk.ptr + blk.size) {
    pos->size += blk.size;
    pos->ptr = blk.ptr;
  }
  else {
    m_list.insert(pos, blk);
  }
}

void gc::free_block_list::insert(void* ptr, size_t size)
{
  free_block blk{size, static_cast<char*>(ptr)};
  auto pos = upper_bound(begin(m_list), end(m_list), blk,
                         [](auto a, auto b) { return a.ptr < b.ptr; });
  m_list.insert(pos, blk);
}
