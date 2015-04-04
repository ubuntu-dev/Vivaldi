#ifndef VV_GC_basic_object_LIST_H
#define VV_GC_basic_object_LIST_H

#include "value.h"

#include <vector>

namespace vv {

namespace gc {

class object_list {
public:
  using iterator = std::vector<gc::managed_ptr>::iterator;

  size_t size() const { return m_list.size(); }

  void push_back(gc::managed_ptr obj) { m_list.push_back(obj); }

  void erase(iterator first, iterator second) { m_list.erase(first, second); }

  iterator begin() { return std::begin(m_list); }
  iterator end()   { return std::end(m_list); }

  ~object_list() { for (auto i : m_list) destruct(i); }

private:
  std::vector<gc::managed_ptr> m_list;
};

}

}

#endif
