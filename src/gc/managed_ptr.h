#ifndef VV_GC_MANAGED_PTR_H
#define VV_GC_MANAGED_PTR_H

#include "block_list.h"

#include <cstdint>
#include <memory>

namespace vv {

namespace gc {

class managed_ptr;

}

namespace value {

struct type;
struct basic_object;

template <typename T>
struct result_type;

template <typename T>
typename result_type<T>::type get(gc::managed_ptr);

}

enum class tag : char {
  nil,
  array,
  array_iterator,
  blob,
  boolean,
  builtin_function,
  character,
  dictionary,
  exception,
  file,
  floating_point,
  function,
  integer,
  method,
  object,
  opt_monop,
  opt_binop,
  partial_function,
  range,
  regex,
  regex_result,
  string,
  string_iterator,
  symbol,
  type,
  environment
};

namespace gc {

class block_list;

template <typename T, typename... Args>
gc::managed_ptr alloc(Args&&... args);

namespace internal {

extern block_list g_blocks;
gc::managed_ptr get_next_empty(tag type, size_t sz);

}

class managed_ptr {
public:
  managed_ptr()
    : m_block  {0},
      m_offset {0},
      m_tag    {tag::nil},
      m_flags  {0}
  { }

  value::basic_object* get() const
  {
    const auto char_ptr = internal::g_blocks.m_list[m_block]->block.data() + m_offset;
    return reinterpret_cast<value::basic_object*>(char_ptr);
  }

  vv::tag tag() const { return m_tag; }
  gc::managed_ptr type() const;

  operator bool() const { return m_flags; }
  operator size_t() const = delete;

private:
  managed_ptr(uint32_t blk, uint16_t os, vv::tag tag, int flags)
    : m_block  {blk},
      m_offset {os},
      m_tag    {tag},
      m_flags  {flags}
  { }

  uint32_t m_block  : 32;
  uint16_t m_offset : 16;
  vv::tag  m_tag    : 8;
  int      m_flags  : 8;

  template <typename T>
  friend typename value::result_type<T>::type value::get(managed_ptr);
  template <typename T, typename... Args>
  friend managed_ptr gc::alloc(Args&&... args);
  friend managed_ptr internal::get_next_empty(vv::tag, size_t);
  friend class gc::block_list;

  friend bool operator==(managed_ptr, managed_ptr) noexcept;
  friend struct std::hash<managed_ptr>;
};

static_assert(sizeof(managed_ptr) == 8, "improper padding in managed_ptr");

bool operator==(managed_ptr lhs, managed_ptr rhs) noexcept;
bool operator!=(managed_ptr lhs, managed_ptr rhs) noexcept;

};

}

template <>
struct std::hash<vv::gc::managed_ptr> {
  size_t operator()(vv::gc::managed_ptr ptr) const;
};

#endif
