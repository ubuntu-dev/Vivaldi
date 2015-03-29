#ifndef VV_GC_ALLOCATED_BLOCK_LIST_H
#define VV_GC_ALLOCATED_BLOCK_LIST_H

#include "value.h"
#include "utils/memory.h"

// Why does the standard not overload std::hash for std::pair again? At least
// someone has the right idea
#include <boost/functional/hash.hpp>

namespace vv {

namespace gc {

free_block_list::value_type get_next_empty(size_t sz);

enum class tag {
  object,
  array,
  array_iterator,
  blob,
  boolean,
  builtin_function,
  dictionary,
  file,
  floating_point,
  function,
  integer,
  nil,
  opt_monop,
  opt_binop,
  range,
  regex,
  regex_result,
  string,
  string_iterator,
  symbol,
  type,
  environment
};

template <typename T>
struct tag_for {};

size_t size_for(tag type);

class allocated_block_list {
public:
  struct value_type {
    value::object* ptr;
    tag tag;
  };
  struct key_compare {
    bool operator()(value_type first, value_type second) const
    {
      return first.ptr < second.ptr;
    };
  };

  using iterator = std::set<value_type>::iterator;
  using const_iterator = std::set<value_type>::const_iterator;

  allocated_block_list() { }

  allocated_block_list(const allocated_block_list& other) = delete;
  allocated_block_list(allocated_block_list&& other) = default;

  allocated_block_list& operator=(const allocated_block_list& other) = delete;
  allocated_block_list& operator=(allocated_block_list&& other) = default;

  bool empty() const { return m_data.empty(); }
  size_t size() const { return m_data.size(); }

  size_t count(value_type ptr) const     { return m_data.count(ptr); }
  size_t count(value::object* ptr) const { return count({ptr, tag::object}); }

  iterator find(value_type ptr)                 { return m_data.find(ptr); }
  const_iterator find(value_type ptr) const     { return m_data.find(ptr); }
  iterator find(value::object* ptr)             { return find({ptr, tag::object}); }
  const_iterator find(value::object* ptr) const { return find({ptr, tag::object}); }

  template <typename T, typename... Args>
  iterator emplace(Args&&... args)
  {
    auto mem = get_next_empty(sizeof(T));
    auto tag = tag_for<T>();
    new (mem.first) T{std::forward<Args>(args)...};
    return m_data.emplace(mem.first, tag);
  }

  iterator insert(value_type ptr) { return m_data.insert(ptr).first; }

  value_type erase(value_type ptr);
  value_type erase(iterator iter);
  value_type erase(value::object* ptr);

  free_block_list::value_type erase_destruct(iterator iter);
  free_block_list::value_type erase_destruct(value_type ptr);
  free_block_list::value_type erase_destruct(value::object* iter);

  void clear() { m_data.clear(); }

  iterator begin()              { return m_data.begin(); }
  const_iterator begin() const  { return cbegin(); }
  const_iterator cbegin() const { return m_data.cbegin(); }

  iterator end()              { return m_data.end(); }
  const_iterator end() const  { return cend(); }
  const_iterator cend() const { return m_data.cend(); }

  ~allocated_block_list();

private:
  std::set<value_type, key_compare> m_data;
};

template <>
struct tag_for<value::object> : std::integral_constant<tag, tag::object> {};
template <>
struct tag_for<value::array> : std::integral_constant<tag, tag::array> {};
template <>
struct tag_for<value::array_iterator> : std::integral_constant<tag, tag::array_iterator> {};
template <>
struct tag_for<value::blob> : std::integral_constant<tag, tag::blob> {};
template <>
struct tag_for<value::boolean> : std::integral_constant<tag, tag::boolean> {};
template <>
struct tag_for<value::builtin_function> : std::integral_constant<tag, tag::builtin_function> {};
template <>
struct tag_for<value::dictionary> : std::integral_constant<tag, tag::dictionary> {};
template <>
struct tag_for<value::file> : std::integral_constant<tag, tag::file> {};
template <>
struct tag_for<value::floating_point> : std::integral_constant<tag, tag::floating_point> {};
template <>
struct tag_for<value::function> : std::integral_constant<tag, tag::function> {};
template <>
struct tag_for<value::integer> : std::integral_constant<tag, tag::integer> {};
template <>
struct tag_for<value::nil> : std::integral_constant<tag, tag::nil> {};
template <>
struct tag_for<value::opt_monop> : std::integral_constant<tag, tag::opt_monop> {};
template <>
struct tag_for<value::opt_binop> : std::integral_constant<tag, tag::opt_binop> {};
template <>
struct tag_for<value::range> : std::integral_constant<tag, tag::range> {};
template <>
struct tag_for<value::regex> : std::integral_constant<tag, tag::regex> {};
template <>
struct tag_for<value::regex_result> : std::integral_constant<tag, tag::regex_result> {};
template <>
struct tag_for<value::string> : std::integral_constant<tag, tag::string> {};
template <>
struct tag_for<value::string_iterator> : std::integral_constant<tag, tag::string_iterator> {};
template <>
struct tag_for<value::symbol> : std::integral_constant<tag, tag::symbol> {};
template <>
struct tag_for<value::type> : std::integral_constant<tag, tag::type> {};
template <>
struct tag_for<vm::environment> : std::integral_constant<tag, tag::environment> {};

}

}

#endif
