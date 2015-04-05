#include "block_list.h"

#include "gc/managed_ptr.h"

#include <boost/optional/optional.hpp>

using namespace vv;
using namespace gc;

// Marking functions {{{

block_list::block_list()
{
  for (auto i = 16; i--;)
    add_new_block();
  m_cur_pos = begin(m_list);
}

bool block_list::is_marked(gc::managed_ptr ptr) const
{
  const auto& blk = m_list[ptr.m_block];
  return blk->markings[ptr.m_offset / 8];
}

void block_list::mark(gc::managed_ptr ptr)
{
  const auto& blk = m_list[ptr.m_block];
  blk->markings.set(ptr.m_offset / 8);
}

void block_list::unmark_all()
{
  for (auto& i : m_list)
    i->markings.reset();
}

// }}}
// Allocation functions {{{

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
boost::optional<char*> get_allocated_space(V& blk,
                                            typename V::iterator& cur_pos,
                                            const size_t sz)
{
  auto it = circular_find(begin(blk), cur_pos, end(blk),
                          [sz](auto blk) { return blk.size >= sz; });

  if (it != end(blk)) {
    const auto ptr = it->data;

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
  return {};
}

}

gc::managed_ptr block_list::allocate(const size_t size)
{
  for (auto i = m_cur_pos; i != end(m_list); ++i) {
    const auto ptr = get_allocated_space((*i)->free_list,
                                         (*i)->free_pos,
                                         size);
    if (ptr) {
      m_cur_pos = i;
      const auto block_idx = i - begin(m_list);
      const auto block_os = *ptr - (*i)->block.data();
      return { static_cast<uint32_t>(block_idx),
               static_cast<uint16_t>(block_os),
               tag::nil, 1 };
    }
  }
  for (auto i = begin(m_list); i != m_cur_pos; ++i) {
    const auto ptr = get_allocated_space((*i)->free_list,
                                         (*i)->free_pos,
                                         size);
    if (ptr) {
      m_cur_pos = i;
      const auto block_idx = i - begin(m_list);
      const auto block_os = *ptr - (*i)->block.data();
      return { static_cast<uint32_t>(block_idx),
               static_cast<uint16_t>(block_os),
               tag::nil, 1 };
    }
  }

  return {};
}

void block_list::reclaim(gc::managed_ptr ptr, const size_t size)
{
  const auto it = begin(m_list) + ptr.m_block;
  auto& list = (*it)->free_list;

  const auto ch_ptr = ptr.m_offset + (*it)->block.data();

  const auto pos = upper_bound(begin(list), end(list), ch_ptr,
                               [](auto* ch_ptr, auto blk) { return ch_ptr < blk.data; });

  if (pos != begin(list)) {
    const auto prev = pos - 1;
    if (prev->data + prev->size == ch_ptr) {
      prev->size += size;
      if (pos != end(list) && pos->data == prev->data + prev->size) {
        prev->size += pos->size;
        (*it)->free_pos = list.erase(pos);
      }
      return;
    }
  }

  if (pos != end(list) && pos->data == ch_ptr + size) {
    pos->size += size;
    pos->data = ch_ptr;
  }
  else {
    (*it)->free_pos = list.insert(pos, {size, ch_ptr});
  }
}

void block_list::expand()
{
  const auto size = m_list.size();
  for (auto i = size / 2; i--;)
    add_new_block();
  m_cur_pos = begin(m_list) + size;
}

void block_list::add_new_block()
{
  auto block = std::make_unique<block_list::block>( );
  block->free_list.push_back({block->block.size(), block->block.data()});
  block->free_pos = begin(block->free_list);

  m_list.emplace_back(move(block));
}

// }}}
