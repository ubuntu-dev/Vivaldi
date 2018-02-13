#ifndef VV_SYMBOL_H
#define VV_SYMBOL_H

#include "gc/managed_ptr.h"

#include <string_view>
#include <unordered_map>

namespace vv {

// Simple symbol class.
// Symbols are, essentially, immutable, cheaply copyable and comparable
// strings. They're used throughout Vivaldi both as a builtin type and for
// variable/member lookup.
class symbol {
public:
  symbol(std::string_view str = "");
  explicit symbol(gc::managed_ptr ptr);

  gc::managed_ptr ptr() const noexcept { return m_ptr; }

  friend bool operator==(symbol first, symbol second) noexcept;

  friend std::string_view to_string(symbol sym);

  static void mark();

private:
  gc::managed_ptr m_ptr;
  static std::unordered_map<std::string_view, gc::managed_ptr> s_symbol_table;

  friend struct std::hash<vv::symbol>;

  template <typename T, typename... Args>
  friend gc::managed_ptr gc::alloc(Args&&...);
};

inline bool operator==(symbol lhs, symbol rhs) noexcept
{
  return lhs.m_ptr == rhs.m_ptr;
}

inline bool operator!=(symbol lhs, symbol rhs) noexcept
{
  return !(lhs == rhs);
}

}

template <>
struct std::hash<vv::symbol> {
  size_t operator()(const vv::symbol& sym) const;
};

#endif
