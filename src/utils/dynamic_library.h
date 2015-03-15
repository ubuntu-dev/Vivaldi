#ifndef VV_UTILS_DYNAMIC_LIBRARY_H
#define VV_UTILS_DYNAMIC_LIBRARY_H

#include <dlfcn.h>

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

  dynamic_library(const dynamic_library& other) = delete;
  dynamic_library(dynamic_library&& other) = delete;
  dynamic_library& operator=(const dynamic_library& other) = delete;
  dynamic_library& operator=(dynamic_library&& other) = delete;

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

  ~dynamic_library()
  {
    dlclose(m_handle);
  }

private:
  void* m_handle;
};

}

#endif
