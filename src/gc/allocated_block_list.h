#ifndef VV_GC_ALLOCATED_BLOCK_LIST_H
#define VV_GC_ALLOCATED_BLOCK_LIST_H

#include "value.h"

#include <unordered_set>

namespace vv {

namespace gc {

class allocated_block_list {
public:
  using value_type = value::object*;

  using iterator = std::unordered_set<value_type>::iterator;
  using const_iterator = std::unordered_set<value_type>::const_iterator;

  allocated_block_list();

  allocated_block_list(const allocated_block_list& other) = delete;
  allocated_block_list(allocated_block_list&& other) = default;

  allocated_block_list& operator=(const allocated_block_list& other) = delete;
  allocated_block_list& operator=(allocated_block_list&& other) = default;

  bool empty() const { return m_data.empty(); }

  size_t size() const { return m_data.size(); }

  size_t count(value_type ptr) const { return m_data.count(ptr); }

  iterator insert(value_type ptr) { return m_data.insert(ptr).first; }

  size_t erase(value_type ptr);
  iterator erase(iterator iter);

  void erase_destruct(iterator iter);
  void erase_destruct(value_type ptr);

  void clear() { m_data.clear(); }

  iterator begin()              { return m_data.begin(); }
  const_iterator begin() const  { return cbegin(); }
  const_iterator cbegin() const { return m_data.cbegin(); }

  iterator end()              { return m_data.end(); }
  const_iterator end() const  { return cend(); }
  const_iterator cend() const { return m_data.cend(); }

  ~allocated_block_list();

private:
  std::unordered_set<value_type> m_data;
};

}

}

#endif
