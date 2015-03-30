#ifndef VV_GC_ALLOCATED_BLOCK_LIST_H
#define VV_GC_ALLOCATED_BLOCK_LIST_H

#include "value.h"
#include "utils/memory.h"

namespace vv {

namespace gc {

free_block_list::value_type get_next_empty(size_t sz);

class allocated_block_list {
public:
  using value_type = value::object*;

  using iterator = std::set<value_type>::iterator;
  using const_iterator = std::set<value_type>::const_iterator;

  allocated_block_list() { }

  allocated_block_list(const allocated_block_list& other) = delete;
  allocated_block_list(allocated_block_list&& other) = default;

  allocated_block_list& operator=(const allocated_block_list& other) = delete;
  allocated_block_list& operator=(allocated_block_list&& other) = default;

  bool empty() const { return m_data.empty(); }
  size_t size() const { return m_data.size(); }

  size_t count(value_type ptr) const     { return m_data.count(ptr); }

  iterator find(value_type ptr)                 { return m_data.find(ptr); }
  const_iterator find(value_type ptr) const     { return m_data.find(ptr); }

  template <typename T, typename... Args>
  iterator emplace(Args&&... args)
  {
    auto mem = get_next_empty(sizeof(T));
    auto tag = tag_for<T>();
    new (mem.first) T{std::forward<Args>(args)...};
    return m_data.emplace(mem.first, tag);
  }

  iterator insert(value_type ptr) { return m_data.insert(ptr).first; }

  value_type erase(value_type ptr);
  value_type erase(iterator iter);

  free_block_list::value_type erase_destruct(iterator iter);
  free_block_list::value_type erase_destruct(value_type ptr);

  void clear() { m_data.clear(); }

  iterator begin()              { return m_data.begin(); }
  const_iterator begin() const  { return cbegin(); }
  const_iterator cbegin() const { return m_data.cbegin(); }

  iterator end()              { return m_data.end(); }
  const_iterator end() const  { return cend(); }
  const_iterator cend() const { return m_data.cend(); }

  ~allocated_block_list();

private:
  std::set<value_type> m_data;
};

}

}

#endif
