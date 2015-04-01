#ifndef VV_GC_OBJECT_LIST_H
#define VV_GC_OBJECT_LIST_H

#include "value.h"

#include <vector>

namespace vv {

namespace gc {

class object_list {
public:
  using iterator = std::vector<value::object*>::iterator;

  size_t size() const { return m_list.size(); }

  void push_back(value::object* obj) { m_list.push_back(obj); }

  void erase(iterator first, iterator second) { m_list.erase(first, second); }

  iterator begin() { return std::begin(m_list); }
  iterator end()   { return std::end(m_list); }

  ~object_list() { for (auto i : m_list) destruct(*i); }

private:
  std::vector<value::object*> m_list;
};

}

}

#endif
