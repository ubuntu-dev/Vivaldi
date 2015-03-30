#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/blob.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/dictionary.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"
#include "value/type.h"

#include <sstream>

using namespace vv;
using namespace value;

size_t vv::size_for(tag type)
{
  switch (type) {
  case tag::object:           return sizeof(object);
  case tag::array:            return sizeof(array);
  case tag::array_iterator:   return sizeof(array_iterator);
  case tag::blob:             return sizeof(blob);
  case tag::boolean:          return sizeof(boolean);
  case tag::builtin_function: return sizeof(builtin_function);
  case tag::dictionary:       return sizeof(dictionary);
  case tag::file:             return sizeof(file);
  case tag::floating_point:   return sizeof(floating_point);
  case tag::function:         return sizeof(function);
  case tag::integer:          return sizeof(integer);
  case tag::nil:              return sizeof(nil);
  case tag::opt_monop:        return sizeof(opt_monop);
  case tag::opt_binop:        return sizeof(opt_binop);
  case tag::range:            return sizeof(range);
  case tag::regex:            return sizeof(regex);
  case tag::regex_result:     return sizeof(regex_result);
  case tag::string:           return sizeof(string);
  case tag::string_iterator:  return sizeof(string_iterator);
  case tag::symbol:           return sizeof(value::symbol);
  case tag::type:             return sizeof(type);
  case tag::environment:      return sizeof(vm::environment);
  }
}

// value_for {{{

namespace {

std::string array_val(const array& arr)
{
  std::ostringstream stm{};
  stm << '[';
  if (arr.val.size()) {
    for_each(begin(arr.val), end(arr.val) - 1,
             [&](const auto& v) { stm << vv::value_for(*v) << ", "; });
    stm << vv::value_for(*arr.val.back());
  }
  stm << ']';
  return stm.str();
}

std::string dictionary_val(const dictionary& dict)
{
  std::string str{"{"};
  for (const auto& pair: dict.val)
    str += ' ' + vv::value_for(*pair.first) += ": " + vv::value_for(*pair.second) += ',';
  if (dict.val.size())
    str.back() = ' ';
  return str += '}';
}

std::string range_val(const range& rng)
{
  return vv::value_for(*rng.start) += " to " + vv::value_for(*rng.end);
}

}

std::string vv::value_for(const object& object)
{
  switch (object.tag) {
  case tag::array:           return array_val(reinterpret_cast<const array&>(object));
  case tag::array_iterator:  return "<array iterator>";
  case tag::boolean:         return reinterpret_cast<const boolean&>(object).val ? "true" : "false";
  case tag::dictionary:      return dictionary_val(reinterpret_cast<const dictionary&>(object));
  case tag::file:            return "File: " + reinterpret_cast<const file&>(object).name;
  case tag::floating_point:  return std::to_string(reinterpret_cast<const floating_point&>(object).val);
  case tag::integer:         return std::to_string(reinterpret_cast<const integer&>(object).val);
  case tag::nil:             return "nil";

  case tag::opt_monop:
  case tag::opt_binop:
  case tag::builtin_function:
  case tag::function:        return "<function>";
  case tag::range:           return range_val(reinterpret_cast<const range&>(object));
  case tag::regex:           return '`' + reinterpret_cast<const regex&>(object).str + '`';
  case tag::regex_result:    return "<regex result>";
  case tag::string:          return '"' + reinterpret_cast<const string&>(object).val + '"';
  case tag::string_iterator: return "<string iterator>";
  case tag::symbol:          return '\'' + to_string(reinterpret_cast<const value::symbol&>(object).val);
  case tag::type:            return to_string(reinterpret_cast<const type&>(object).name);

  case tag::blob:
  case tag::environment:
  case tag::object:          return "<object>";
  }
}

// }}}
// hash_for {{{

namespace {

template <typename T>
size_t hash_val(const T& item)
{
  return std::hash<decltype(item.val)>{}(item.val);
}

}

size_t vv::hash_for(const value::object& obj)
{
  switch (obj.tag) {
  case tag::boolean:        return hash_val(static_cast<const boolean&>(obj));
  case tag::floating_point: return hash_val(static_cast<const floating_point&>(obj));
  case tag::integer:        return hash_val(static_cast<const integer&>(obj));
  case tag::string:         return hash_val(static_cast<const string&>(obj));
  case tag::symbol:         return hash_val(static_cast<const value::symbol&>(obj));
  default:                  return std::hash<const value::object*>{}(&obj);
  }
}

// }}}
// equals {{{

namespace {

template <typename T>
bool val_equals(const value::object& first, const value::object& second)
{
  return static_cast<const T&>(first).val == static_cast<const T&>(second).val;
}

}

bool vv::equals(const value::object& first, const value::object& second)
{
  if (&first == &second)
    return true;
  if (first.tag != second.tag)
    return false;

  switch (first.tag) {
  case tag::boolean:        return val_equals<boolean>(first, second);
  case tag::floating_point: return val_equals<floating_point>(first, second);
  case tag::integer:        return val_equals<integer>(first, second);
  case tag::string:         return val_equals<string>(first, second);
  case tag::symbol:         return val_equals<value::symbol>(first, second);
  default:                  return false;
  }
}

// }}}
// destruct {{{

namespace {

template <typename T>
void call_dtor(T& obj)
{
  obj.~T();
}

}

void vv::destruct(object& obj)
{
  switch (obj.tag) {
  case tag::object:           return call_dtor(obj);
  case tag::array:            return call_dtor(static_cast<array&>(obj));
  case tag::array_iterator:   return call_dtor(static_cast<array_iterator&>(obj));
  case tag::blob:             return call_dtor(static_cast<blob&>(obj));
  case tag::boolean:          return call_dtor(static_cast<boolean&>(obj));
  case tag::builtin_function: return call_dtor(static_cast<builtin_function&>(obj));
  case tag::dictionary:       return call_dtor(static_cast<dictionary&>(obj));
  case tag::file:             return call_dtor(static_cast<file&>(obj));
  case tag::floating_point:   return call_dtor(static_cast<floating_point&>(obj));
  case tag::function:         return call_dtor(static_cast<function&>(obj));
  case tag::integer:          return call_dtor(static_cast<integer&>(obj));
  case tag::nil:              return call_dtor(static_cast<nil&>(obj));
  case tag::opt_monop:        return call_dtor(static_cast<opt_monop&>(obj));
  case tag::opt_binop:        return call_dtor(static_cast<opt_binop&>(obj));
  case tag::range:            return call_dtor(static_cast<range&>(obj));
  case tag::regex:            return call_dtor(static_cast<regex&>(obj));
  case tag::regex_result:     return call_dtor(static_cast<regex_result&>(obj));
  case tag::string:           return call_dtor(static_cast<string&>(obj));
  case tag::string_iterator:  return call_dtor(static_cast<string_iterator&>(obj));
  case tag::symbol:           return call_dtor(static_cast<value::symbol&>(obj));
  case tag::type:             return call_dtor(static_cast<type&>(obj));
  case tag::environment:      return call_dtor(static_cast<vm::environment&>(obj));
  }
}

// }}}
