#ifndef VV_GC_MANAGED_PTR_H
#define VV_GC_MANAGED_PTR_H

#include <cstddef>

// Forward declarations {{{

namespace vv {

namespace value {

struct object;

}

namespace gc {

template <typename T>
class managed_ptr;

namespace internal {

struct block;

managed_ptr<value::object> get_next_empty();

}

}

}

// }}}

namespace vv {

namespace gc {

template <typename T>
class managed_ptr {
public:
  managed_ptr(std::nullptr_t = nullptr) : m_ptr{nullptr}, m_blk{nullptr} { }
  managed_ptr(T* ptr, internal::block& blk) : m_ptr{ptr}, m_blk{&blk} { }
  managed_ptr(T* ptr) : m_ptr{ptr}, m_blk{nullptr} { }

  T& operator*() const { return *m_ptr; }

  T* operator->() const { return m_ptr; }

  T* get() const { return m_ptr; }

  operator bool() const { return m_ptr; }

  bool has_block() const { return m_blk; }
  internal::block& block() { return *m_blk; }

  template <typename O>
  operator managed_ptr<O>() const { return {static_cast<O*>(m_ptr), *m_blk}; }

  bool operator==(const managed_ptr& other) const
  {
    return get() == other.get();
  }

  bool operator!=(const managed_ptr& other) const
  {
    return !(*this == other);
  }

private:
  T* m_ptr;
  internal::block* m_blk;

  friend managed_ptr<value::object> internal::get_next_empty();
  template <typename O>
  friend class managed_ptr;
  friend void mark(managed_ptr<value::object>);
};

}

}

#endif
