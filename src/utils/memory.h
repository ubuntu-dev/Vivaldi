#ifndef VV_UTILS_MEMORY_H
#define VV_UTILS_MEMORY_H

#include <iterator>
#include <set>

namespace vv {

template <typename C>
bool contains(const C& cont, const typename C::value_type* ptr)
{
  return ptr >= cont.data() && std::distance(cont.data(), ptr) < cont.size();
}

template <typename T>
std::pair<char*, size_t> get_block_info(const T& item)
{
  return {&item, sizeof(T)};
}

template <typename T>
T* align(T* unaligned)
{
  auto ptr = reinterpret_cast<size_t>(unaligned);
  if (ptr % sizeof(void*) == 0)
    return unaligned;
  return reinterpret_cast<T*>(((ptr / sizeof(void*)) + 1) * sizeof(void*));
}

class free_block_list {
public:
  using value_type = std::pair<char*, size_t>;

  using iterator = std::set<value_type>::iterator;
  using const_iterator = std::set<value_type>::const_iterator;
  using reverse_iterator = std::set<value_type>::reverse_iterator;
  using const_reverse_iterator = std::set<value_type>::const_reverse_iterator;

  bool empty() const { return m_data.empty(); }
  size_t size() const { return m_data.size(); }

  size_t count(value_type ptr) const { return m_data.count(ptr); }

  iterator find(value_type ptr)             { return m_data.find(ptr); }
  const_iterator find(value_type ptr) const { return m_data.find(ptr); }

  iterator insert(value_type ptr);

  iterator erase(value_type ptr) { return m_data.erase(find(ptr)); }
  iterator erase(iterator iter)  { return m_data.erase(iter); }

  iterator begin()                       { return m_data.begin(); }
  const_iterator begin() const           { return cbegin(); }
  const_iterator cbegin() const          { return m_data.cbegin(); }
  reverse_iterator rbegin()              { return m_data.rbegin(); }
  const_reverse_iterator rbegin() const  { return crbegin(); }
  const_reverse_iterator crbegin() const { return m_data.crbegin(); }

  iterator end()                       { return m_data.end(); }
  const_iterator end() const           { return cend(); }
  const_iterator cend() const          { return m_data.cend(); }
  reverse_iterator rend()              { return m_data.rend(); }
  const_reverse_iterator rend() const  { return crend(); }
  const_reverse_iterator crend() const { return m_data.crend(); }

private:
  std::set<value_type> m_data;
};

}

#endif
