#ifndef VV_VALUE_H
#define VV_VALUE_H

#include "symbol.h"
#include "gc/managed_ptr.h"

#include <string>
#include <utility>

namespace vv {

// Classes representing Vivaldi objects.
namespace value {

struct basic_object;

struct array;
struct array_iterator;
struct blob;
struct builtin_function;
struct dictionary;
struct exception;
struct file;
struct floating_point;
struct function;
struct method;
struct object;
struct opt_monop;
struct opt_binop;
struct partial_function;
struct range;
struct regex;
struct regex_result;
struct string;
struct string_iterator;
struct symbol;
struct type;
using boolean = bool;
using character = char;
using integer = int64_t;
using nil = void;

template <typename T>
struct result_type {
  using type = typename T::value_type&;
};

template <>
struct result_type<integer> {
  using type = integer;
};

template <>
struct result_type<boolean> {
  using type = boolean;
};

template <>
struct result_type<character> {
  using type = character;
};

template <typename T>
inline typename result_type<T>::type get(gc::managed_ptr ptr)
{
  return static_cast<T*>(ptr.get())->value;
}

template <>
inline result_type<boolean>::type get<boolean>(gc::managed_ptr ptr)
{
  return static_cast<bool>(ptr.m_block);
}

template <>
inline result_type<character>::type get<character>(gc::managed_ptr ptr)
{
  return static_cast<char>(ptr.m_block);
}

template <>
inline result_type<integer>::type get<integer>(gc::managed_ptr ptr)
{
  // first cast m_block to int32_t so the cast to int64_t knows to pad with the
  // sign bit, not just 0s. m_offset *should* be padded with zeros, so cast
  // directly from uint16_t to int64_t.
  return (static_cast<int64_t>(static_cast<int32_t>(ptr.m_block)) << 16) |
          static_cast<int64_t>(ptr.m_offset);
}

}

// Odd duck--- kept as an object for GC purposes and because it overlaps a lot
// (objects, call frames, and types are all at their core just hash tables
// mapping symbols to objects)
namespace vm {

struct environment;

}

std::string value_for(gc::managed_ptr object);
size_t hash_for(gc::managed_ptr object);
bool equals(gc::managed_ptr lhs, gc::managed_ptr rhs);

void destroy(gc::managed_ptr object);

bool has_member(gc::managed_ptr object, vv::symbol name);
gc::managed_ptr get_member(gc::managed_ptr object, vv::symbol name);
void set_member(gc::managed_ptr object, vv::symbol name, gc::managed_ptr mem);
void clear_members(gc::managed_ptr object);

void mark_members(gc::managed_ptr object);

gc::managed_ptr get_method(gc::managed_ptr type, vv::symbol name);

size_t size_for(tag type);

template <typename T>
struct tag_for {};

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
struct tag_for<value::character> : std::integral_constant<tag, tag::character> {};
template <>
struct tag_for<value::dictionary> : std::integral_constant<tag, tag::dictionary> {};
template <>
struct tag_for<value::exception> : std::integral_constant<tag, tag::exception> {};
template <>
struct tag_for<value::file> : std::integral_constant<tag, tag::file> {};
template <>
struct tag_for<value::floating_point> : std::integral_constant<tag, tag::floating_point> {};
template <>
struct tag_for<value::function> : std::integral_constant<tag, tag::function> {};
template <>
struct tag_for<value::integer> : std::integral_constant<tag, tag::integer> {};
template <>
struct tag_for<value::method> : std::integral_constant<tag, tag::method> {};
template <>
struct tag_for<value::opt_monop> : std::integral_constant<tag, tag::opt_monop> {};
template <>
struct tag_for<value::opt_binop> : std::integral_constant<tag, tag::opt_binop> {};
template <>
struct tag_for<value::nil> : std::integral_constant<tag, tag::nil> {};
template <>
struct tag_for<value::partial_function> : std::integral_constant<tag, tag::partial_function> {};
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

#endif
