#ifndef VV_UTILS_DYNAMIC_LIBRARY_H
#define VV_UTILS_DYNAMIC_LIBRARY_H

#include <dlfcn.h>

#include <functional>
#include <string>
#include <stdexcept>

namespace vv {

class dylib_error : public std::runtime_error {
public:
  dylib_error() : std::runtime_error{dlerror()} { }
};

class dynamic_library {
public:
  dynamic_library(const std::string& filename, int mode = RTLD_LAZY)
    : m_handle {dlopen(filename.c_str(), mode)}
  {
    if (!m_handle)
      throw dylib_error{};
  }

  dynamic_library() noexcept
    : m_handle {nullptr}
  { }

  dynamic_library(const dynamic_library& other) = delete;

  dynamic_library(dynamic_library&& other) noexcept
    : m_handle {other.m_handle}
  {
    other.m_handle = nullptr;
  }

  dynamic_library& operator=(const dynamic_library& other) = delete;

  dynamic_library& operator=(dynamic_library&& other) noexcept
  {
    std::swap(m_handle, other.m_handle);
    return *this;
  }

  template <typename T>
  T get_sym(const char* symbol)
  {
    auto* sym = dlsym(m_handle, symbol);
    if (!sym)
      throw dylib_error{};
    return *reinterpret_cast<T*>(&sym);
  }

  template <typename T>
  T get_sym(const std::string& symbol)
  {
    return get_sym<T>(symbol.c_str());
  }

  template <typename T>
  std::function<T> get_fn(const char* symbol)
  {
    auto* sym = dlsym(m_handle, symbol);
    if (!sym)
      throw dylib_error{};
    return **reinterpret_cast<T**>(&sym);
  }

  template <typename T>
  std::function<T> get_fn(const std::string& symbol)
  {
    return get_fn<T>(symbol.c_str());
  }

  ~dynamic_library()
  {
    if (m_handle)
      dlclose(m_handle);
  }

private:
  void* m_handle;
};

}

#endif
