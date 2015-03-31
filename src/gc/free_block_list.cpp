#include "free_block_list.h"

using namespace vv;

namespace {

const auto blk_less = [](const auto& first, const auto& second)
{
  return first.ptr < second.ptr;
};

template <typename I, typename P>
I circular_find(const I& begin, const I& start, const I& end, const P& pred)
{
  auto i = find_if(start, end, pred);
  if (i == end) {
    i = find_if(begin, start, pred);
    if (i == start)
      return end;
  }
  return i;
}

template <typename V>
void* get_allocated_space(V& blk, typename V::iterator& cur_pos, size_t sz)
{
  auto it = circular_find(begin(blk), cur_pos, end(blk),
                          [sz](auto blk) { return blk.size >= sz; });

  if (it != end(blk)) {
    auto ptr = it->ptr;

    if (it->size > sz) {
      it->ptr += sz;
      it->size -= sz;
    }
    else {
      // whoops
      it = blk.erase(it);
    }

    cur_pos = it;
    return ptr;
  }
  return nullptr;
}

}

void* gc::free_block_list::allocate(size_t sz)
{
  for (auto i = m_cur_pos; i != end(m_list); ++i) {
    auto ptr = get_allocated_space(i->blk, i->cur_pos, sz);
    if (ptr) {
      m_cur_pos = i;
      return ptr;
    }
  }
  for (auto i = begin(m_list); i != m_cur_pos; ++i) {
    auto ptr = get_allocated_space(i->blk, i->cur_pos, sz);
    if (ptr) {
      m_cur_pos = i;
      return ptr;
    }
  }

  return nullptr;
}

void gc::free_block_list::reclaim(void* ptr, size_t size)
{
  free_block blk{size, static_cast<char*>(ptr)};
  auto it = --upper_bound(begin(m_list), end(m_list), blk, blk_less);
  auto& list = it->blk;

  auto pos = upper_bound(begin(list), end(list), blk, blk_less);

  if (pos != begin(list)) {
    auto prev = pos - 1;
    if (prev->ptr + prev->size == blk.ptr) {
      prev->size += blk.size;
      if (pos != end(list) && pos->ptr == prev->ptr + prev->size) {
        prev->size += pos->size;
        auto tmp = list.erase(pos);
        it->cur_pos = tmp;
      }
      return;
    }
  }

  if (pos != end(list) && pos->ptr == blk.ptr + blk.size) {
    pos->size += blk.size;
    pos->ptr = blk.ptr;
  }
  else {
    auto tmp = list.insert(pos, blk);
    it->cur_pos = tmp;
  }
}

void gc::free_block_list::insert(void* ptr, size_t size)
{
  super_block blk{size, static_cast<char*>(ptr)};

  auto pos = upper_bound(begin(m_list), end(m_list), blk, blk_less);
  m_cur_pos = m_list.insert(pos, std::move(blk));
}

gc::free_block_list::super_block::super_block(size_t sz, char* p)
  : size    {sz},
    ptr     {p},
    blk     {{size, ptr}},
    cur_pos {begin(blk)}
{ }
