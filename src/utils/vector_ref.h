#ifndef VV_UTILS_VECTOR_REF_H
#define VV_UTILS_VECTOR_REF_H

#include <vector>
#include <boost/utility/string_ref.hpp>

namespace vv {

// Dumping ground for random, non-component-specific functions, classes, etc.

// The vector_ref is basically analogous to boost::string_ref (or
// std::string_view come C++17, but 'ref' is shorter); it provides a non-owning,
// cheaply copyable handle to a vector. Crucially, the beginning can be lopped
// off of a vector_ref simply by incrementing a pointer.
//
// Alternately, you could think of it as a smart (well, sort of) pointer with an
// associated size.
template <typename T>
class vector_ref {
public:
  using iterator = const T*;

  const static size_t npos{std::numeric_limits<size_t>::max()};

  vector_ref(const std::vector<T>& vec)
    : m_data {vec.data()},
      m_sz   {vec.size()}
  { }

  template <size_t N>
  vector_ref(const std::array<T, N>& vec)
    : m_data {vec.data()},
      m_sz   {N}
  { }

  template <typename I>
  vector_ref(I first, I last)
  : m_data {&*first},
    m_sz   ( static_cast<size_t>(last - first) )
  { }

  vector_ref() : m_data{nullptr}, m_sz{0} { }

  vector_ref subvec(size_t front, size_t back = npos) const
  {
    return {m_data + front, m_data + (back == npos ? m_sz : back)};
  }

  // Can be used to move the beginning of the vector_ref *backwards*. Don't use
  // this unless you're absolutely certain there's valid data there! As a rule,
  // don't ever use this function with a negative offset except to counteract a
  // previous positive offset.
  vector_ref shifted_by(long offset) const
  {
    return {m_data + offset, m_data + m_sz};
  }

  // Imitation vector functions
  const T& front() const { return *m_data; }
  const T& back() const { return m_data[m_sz - 1]; }

  size_t size() const { return m_sz; }

  const T& operator[](int idx) { return m_data[idx]; }

  iterator begin() const { return m_data; }
  iterator end() const { return m_data + m_sz; }

  const T* data() const { return m_data; }

private:
  const T* m_data;
  size_t m_sz;
};

// Given a vector_ref vec, return a vector_ref pointing to the first element of
// vec not equal to item
template <typename T>
inline vector_ref<T> ltrim(vector_ref<T> vec, const T& item)
{
  auto last = find_if(begin(vec), end(vec),
                      [&](const auto& i) { return i != item; });
  return vec.subvec(static_cast<size_t>(last - begin(vec)));
}

// Given a vector_ref vec, returns a vector_ref pointing to the first element e
// of vec such that pred(e) returns false
template <typename T, typename F>
inline vector_ref<T> ltrim_if(vector_ref<T> vec, const F& pred)
{
  auto last = find_if_not(begin(vec), end(vec), pred);
  return vec.subvec(static_cast<size_t>(last - begin(vec)));
}

}

#endif
