#ifndef VV_VALUE_H
#define VV_VALUE_H

#include <string>
#include <utility>

namespace vv {

// Classes representing Vivaldi objects.
namespace value {

struct array;
struct array_iterator;
struct blob;
struct boolean;
struct builtin_function;
struct dictionary;
struct file;
struct floating_point;
struct function;
struct integer;
struct nil;
struct object;
struct opt_monop;
struct opt_binop;
struct range;
struct regex;
struct regex_result;
struct string;
struct string_iterator;
struct symbol;
struct type;

}

// Odd duck--- kept as an object for GC purposes and because it overlaps a lot
// (objects, call frames, and types are all at their core just hash tables
// mapping symbols to objects)
namespace vm {

struct environment;

}

std::string value_for(const value::object& object);
size_t hash_for(const value::object& object);
bool equals(const value::object& first, const value::object& second);
void destruct(value::object& object);

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

#endif
