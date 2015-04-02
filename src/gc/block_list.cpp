#include "block_list.h"

using namespace vv;
using namespace gc;

// Marking functions {{{

block_list::block_list()
{
  for (auto i = 4; i--;)
    add_new_block();
}

bool block_list::contains(void* ptr) const
{
  const auto ch_ptr = static_cast<char*>(ptr);
  auto iter = m_list.upper_bound(ch_ptr);
  if (iter == begin(m_list))
    return false;

  --iter;
  if (iter->first + iter->second->block.size() <= ptr)
    return false;
  return true;
}

bool block_list::is_marked(void* ptr) const
{
  const auto ch_ptr = static_cast<char*>(ptr);
  const auto iter = --m_list.upper_bound(ch_ptr);
  const auto dist = ch_ptr - iter->first;
  return iter->second->markings[dist / 8];
}

void block_list::mark(void* ptr)
{
  const auto ch_ptr = static_cast<char*>(ptr);
  const auto iter = --m_list.upper_bound(ch_ptr);
  const auto dist = ch_ptr - iter->first;
  iter->second->markings.set(dist / 8);
}

void block_list::unmark_all()
{
  for (auto& i : m_list)
    i.second->markings.reset();
}

// }}}
// Allocation functions {{{
//
namespace {

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
void* get_allocated_space(V& blk, typename V::iterator& cur_pos, const size_t sz)
{
  auto it = circular_find(begin(blk), cur_pos, end(blk),
                          [sz](auto blk) { return blk.size >= sz; });

  if (it != end(blk)) {
    auto ptr = it->data;

    if (it->size > sz) {
      it->data += sz;
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

void* block_list::allocate(const size_t size)
{
  for (auto i = m_cur_pos; i != end(m_list); ++i) {
    const auto ptr = get_allocated_space(i->second->free_list,
                                         i->second->free_pos,
                                         size);
    if (ptr) {
      m_cur_pos = i;
      return ptr;
    }
  }
  for (auto i = begin(m_list); i != m_cur_pos; ++i) {
    const auto ptr = get_allocated_space(i->second->free_list,
                                         i->second->free_pos,
                                         size);
    if (ptr) {
      m_cur_pos = i;
      return ptr;
    }
  }

  return nullptr;
}

void block_list::reclaim(void* ptr, const size_t size)
{
  const auto ch_ptr = static_cast<char*>(ptr);
  const auto it = --m_list.upper_bound(ch_ptr);
  auto& list = it->second->free_list;

  const auto pos = upper_bound(begin(list), end(list), ptr,
                               [](auto* ptr, auto blk) { return ptr < blk.data; });

  if (pos != begin(list)) {
    const auto prev = pos - 1;
    if (prev->data + prev->size == ch_ptr) {
      prev->size += size;
      if (pos != end(list) && pos->data == prev->data + prev->size) {
        prev->size += pos->size;
        it->second->free_pos = list.erase(pos);
      }
      return;
    }
  }

  if (pos != end(list) && pos->data == ch_ptr + size) {
    pos->size += size;
    pos->data = ch_ptr;
  }
  else {
    it->second->free_pos = list.insert(pos, {size, ch_ptr});
  }
}

void block_list::expand()
{
  for (auto i = m_list.size() / 2; i--;)
    add_new_block();
}

void block_list::add_new_block()
{
  auto block = std::make_unique<block_list::block>( );
  block->free_list.push_back({block->block.size(), block->block.data()});
  block->free_pos = begin(block->free_list);

  m_cur_pos = m_list.emplace(block->block.data(), move(block)).first;
}

// }}}
