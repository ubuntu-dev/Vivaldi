#ifndef VV_UTILS_DUMB_PTR_H
#define VV_UTILS_DUMB_PTR_H

namespace vv {

// Forms a dumb, non-owning 'smart' pointer, essentially identical to a regular
// pointer sans arithmetic.
template <typename T>
class dumb_ptr {
public:
  dumb_ptr(T* ptr = nullptr) : m_ptr{ptr} { }

  T* get() const { return m_ptr; }

  T& operator*() const { return *m_ptr; }
  operator bool() const { return m_ptr; }
  T* operator->() const { return m_ptr; }

  template <typename O>
  operator dumb_ptr<O>() const { return static_cast<O>(m_ptr); }

private:
  T* m_ptr;
};

}

#endif
