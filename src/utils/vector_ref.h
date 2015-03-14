#ifndef VV_UTILS_VECTOR_REF_H
#define VV_UTILS_VECTOR_REF_H

#include <vector>
#include <boost/utility/string_ref.hpp>

namespace vv {

/// Simple immutable, non-owning vector class.

/// The vector_ref is basically analogous to boost::string_ref (or
/// std::string_view come C++17, but 'ref' is shorter); it provides a non-owning,
/// cheaply copyable handle to a vector. Crucially, the beginning can be lopped
/// off of a vector_ref simply by incrementing a pointer.
///
/// Alternately, you could think of it as a smart (well, sort of) pointer with an
/// associated size.
template <typename T>
class vector_ref {
public:
  using iterator = const T*;

  const static size_t npos{std::numeric_limits<size_t>::max()};

  /// Constructs a vector_ref pointing to the provided vector.
  vector_ref(const std::vector<T>& vec)
    : m_data {vec.data()},
      m_sz   {vec.size()}
  { }

  /// Constructs a vector_ref pointing to the provided array.
  template <size_t N>
  vector_ref(const std::array<T, N>& vec)
    : m_data {vec.data()},
      m_sz   {N}
  { }

  /// Constructs a vector_ref pointing to the contiguous region of memory
  /// starting at `first` and ending with `last`.
  template <typename I>
  vector_ref(I first, I last)
  : m_data {&*first},
    m_sz   ( static_cast<size_t>(last - first) )
  { }

  /// Constructs a default, empty vector_ref.
  vector_ref() : m_data{nullptr}, m_sz{0} { }

  /// A vector_ref offset by `front` and `back`.
  vector_ref subvec(size_t front, size_t back = npos) const
  {
    return {m_data + front, m_data + (back == npos ? m_sz : back)};
  }

  /// Can be used to move the beginning of the vector_ref *backwards*.

  /// Don't event think about using this unless you're absolutely certain
  /// there's valid data there! As a rule, don't ever use this function with a
  /// negative offset except to counteract a previous positive offset.
  vector_ref shifted_by(long offset) const
  {
    return {m_data + offset, m_data + m_sz};
  }

  /// Constant reference to the first item in the vector_ref.
  const T& front() const { return *m_data; }
  /// Constant reference to the last item in the vector_ref.
  const T& back() const { return m_data[m_sz - 1]; }

  /// The number of elements in the vector_ref.
  size_t size() const { return m_sz; }

  /// A constant reference to the `idx`th element of the vector_ref.
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
  auto last = std::find_if(std::begin(vec), std::end(vec),
                           [&](const auto& i) { return i != item; });
  return vec.subvec(static_cast<size_t>(last - std::begin(vec)));
}

// Given a vector_ref vec, returns a vector_ref pointing to the first element e
// of vec such that pred(e) returns false
template <typename T, typename F>
inline vector_ref<T> ltrim_if(vector_ref<T> vec, const F& pred)
{
  auto last = std::find_if_not(std::begin(vec), std::end(vec), pred);
  return vec.subvec(static_cast<size_t>(last - std::begin(vec)));
}

}

#endif
