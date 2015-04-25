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

enum class tag {
  nil               = 0b00000,
  array             = 0b00001,
  array_iterator    = 0b00010,
  blob              = 0b00011,
  boolean           = 0b00100,
  builtin_function  = 0b00101,
  character         = 0b00110,
  dictionary        = 0b00111,
  file              = 0b01000,
  floating_point    = 0b01001,
  function          = 0b01010,
  integer           = 0b01011,
  method            = 0b01100,
  object            = 0b01101,
  opt_monop         = 0b01110,
  opt_binop         = 0b01111,
  partial_function  = 0b10000,
  range             = 0b10001,
  regex             = 0b10010,
  regex_result      = 0b10011,
  string            = 0b10100,
  string_iterator   = 0b10101,
  symbol            = 0b10110,
  type              = 0b10111,
  environment       = 0b11000,
};

namespace gc {

class block_list;

template <typename T, typename... Args>
gc::managed_ptr alloc(Args&&... args);

namespace internal {

extern block_list g_blocks;
gc::managed_ptr get_next_empty(tag type);

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
  friend managed_ptr internal::get_next_empty(vv::tag);
  friend class gc::block_list;

  friend bool operator==(managed_ptr, managed_ptr);
  friend struct std::hash<managed_ptr>;
};

static_assert(sizeof(managed_ptr) == 8, "improper padding in managed_ptr");

bool operator==(managed_ptr lhs, managed_ptr rhs);
bool operator!=(managed_ptr lhs, managed_ptr rhs);

};

}

template <>
struct std::hash<vv::gc::managed_ptr> {
  size_t operator()(vv::gc::managed_ptr ptr) const;
};

#endif
