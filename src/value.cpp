#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "utils/lang.h"
#include "utils/string_helpers.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/blob.h"
#include "value/builtin_function.h"
#include "value/dictionary.h"
#include "value/exception.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/method.h"
#include "value/object.h"
#include "value/opt_functions.h"
#include "value/partial_function.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"
#include "value/type.h"

#include <sstream>

using namespace vv;
using namespace value;

namespace {

// Global value, used to store instance variables for non-value::object classes
std::unordered_map<gc::managed_ptr,
                   hash_map<vv::symbol, gc::managed_ptr>> g_generic_members;

}

size_t vv::size_for(const tag type)
{
  switch (type) {
  case tag::object:           return sizeof(value::object);
  case tag::array:            return sizeof(value::array);
  case tag::array_iterator:   return sizeof(value::array_iterator);
  case tag::blob:             return sizeof(value::blob);
  case tag::boolean:          return sizeof(value::boolean);
  case tag::builtin_function: return sizeof(value::builtin_function);
  case tag::character:        return sizeof(value::character);
  case tag::dictionary:       return sizeof(value::dictionary);
  case tag::exception:        return sizeof(value::exception);
  case tag::file:             return sizeof(value::file);
  case tag::floating_point:   return sizeof(value::floating_point);
  case tag::function:         return sizeof(value::function);
  case tag::integer:          return sizeof(value::integer);
  case tag::method:           return sizeof(value::method);
  case tag::nil:              return 0;
  case tag::opt_monop:        return sizeof(value::opt_monop);
  case tag::opt_binop:        return sizeof(value::opt_binop);
  case tag::partial_function: return sizeof(value::partial_function);
  case tag::range:            return sizeof(value::range);
  case tag::regex:            return sizeof(value::regex);
  case tag::regex_result:     return sizeof(value::regex_result);
  case tag::string:           return sizeof(value::string);
  case tag::string_iterator:  return sizeof(value::string_iterator);
  case tag::symbol:           return sizeof(vv::symbol);
  case tag::type:             return sizeof(value::type);
  case tag::environment:      return sizeof(vm::environment);
  }
}

// value_for {{{

namespace {

std::string array_val(const array::value_type& arr)
{
  std::ostringstream stm{};
  stm << '[';
  if (!arr.empty()) {
    for_each(begin(arr), end(arr) - 1,
             [&](const auto& v) { stm << vv::value_for(v) << ", "; });
    stm << vv::value_for(arr.back());
  }
  stm << ']';
  return stm.str();
}

std::string dictionary_val(const dictionary::value_type& dict)
{
  std::string str{"{"};
  for (const auto& pair: dict)
    str += ' ' + vv::value_for(pair.first) += ": " + vv::value_for(pair.second) += ',';
  if (!dict.empty())
    str.back() = ' ';
  return str += '}';
}

std::string exception_val(gc::managed_ptr exception)
{
  const auto str = value_for(exception.type());
  const auto& exc_str = get<value::exception>(exception).message;
  if (exc_str.empty())
    return str;

  return str + ": " + exc_str;
}

std::string range_val(const range::value_type& rng)
{
  return vv::value_for(rng.start) += " to " + vv::value_for(rng.end);
}

}

std::string vv::value_for(gc::managed_ptr ptr)
{
  switch (ptr.tag()) {
  case tag::array:           return array_val(get<array>(ptr));
  case tag::array_iterator:  return "<array iterator>";
  case tag::boolean:         return get<boolean>(ptr) ? "true" : "false";
  case tag::character:       return get_escaped_name(get<character>(ptr));
  case tag::dictionary:      return dictionary_val(get<dictionary>(ptr));
  case tag::exception:       return exception_val(ptr);
  case tag::file:            return "File: " + get<file>(ptr).name;
  case tag::floating_point:  return std::to_string(get<floating_point>(ptr));
  case tag::integer:         return std::to_string(get<integer>(ptr));
  case tag::nil:             return "nil";

  case tag::opt_monop:
  case tag::opt_binop:
  case tag::builtin_function:
  case tag::method:
  case tag::partial_function:
  case tag::function:        return "<function>";
  case tag::range:           return range_val(get<range>(ptr));
  case tag::regex:           return '`' + get<regex>(ptr).str + '`';
  case tag::regex_result:    return "<regex result>";
  case tag::string:          return '"' + escape_chars(get<string>(ptr)) + '"';
  case tag::string_iterator: return "<string iterator>";
  case tag::symbol:          return '\'' + std::string{to_string(get<value::symbol>(ptr))};
  case tag::type:            return std::string{to_string(get<type>(ptr).name)};
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
  return std::hash<T>{}(item);
}

}

size_t vv::hash_for(gc::managed_ptr obj)
{
  switch (obj.tag()) {
  case tag::boolean:        return hash_val(get<boolean>(obj));
  case tag::character:      return hash_val(get<character>(obj));
  case tag::floating_point: return hash_val(get<floating_point>(obj));
  case tag::integer:        return hash_val(get<integer>(obj));
  case tag::string:         return hash_val(get<string>(obj));
  case tag::symbol:         return hash_val(get<value::symbol>(obj));
  default:                  return std::hash<gc::managed_ptr>{}(obj);
  }
}

// }}}
// equals {{{

namespace {

template <typename T>
bool val_equals(gc::managed_ptr lhs, gc::managed_ptr rhs)
{
  return get<T>(lhs) == get<T>(rhs);
}

}

bool vv::equals(gc::managed_ptr lhs, gc::managed_ptr rhs)
{
  if (lhs == rhs)
    return true;
  if (lhs.tag() != rhs.tag())
    return false;

  switch (lhs.tag()) {
  case tag::boolean:        return val_equals<boolean>(lhs, rhs);
  case tag::character:      return val_equals<character>(lhs, rhs);
  case tag::floating_point: return val_equals<floating_point>(lhs, rhs);
  case tag::integer:        return val_equals<integer>(lhs, rhs);
  case tag::string:         return val_equals<string>(lhs, rhs);
  case tag::symbol:         return val_equals<value::symbol>(lhs, rhs);
  default:                  return false;
  }
}

// }}}
// destroy {{{

namespace {

template <typename T>
void call_dtor(T& obj)
{
  obj.~T();
}

}

// TODO: figure out why this is taking up any runtime since it's the least
// computationally expensive thing imaginable. Maybe it's just because of the
// lack of inlining in debug builds?
void vv::destroy(gc::managed_ptr obj)
{
  switch (obj.tag()) {
  case tag::object:           call_dtor(*obj.get());                                 break;
  case tag::array:            call_dtor(static_cast<array&>(*obj.get()));            break;
  case tag::array_iterator:   call_dtor(static_cast<array_iterator&>(*obj.get()));   break;
  case tag::blob:             call_dtor(static_cast<blob&>(*obj.get()));             break;
  case tag::builtin_function: call_dtor(static_cast<builtin_function&>(*obj.get())); break;
  case tag::dictionary:       call_dtor(static_cast<dictionary&>(*obj.get()));       break;
  case tag::file:             call_dtor(static_cast<file&>(*obj.get()));             break;
  case tag::floating_point:   call_dtor(static_cast<floating_point&>(*obj.get()));   break;
  case tag::function:         call_dtor(static_cast<function&>(*obj.get()));         break;
  case tag::opt_monop:        call_dtor(static_cast<opt_monop&>(*obj.get()));        break;
  case tag::opt_binop:        call_dtor(static_cast<opt_binop&>(*obj.get()));        break;
  case tag::range:            call_dtor(static_cast<range&>(*obj.get()));            break;
  case tag::regex:            call_dtor(static_cast<regex&>(*obj.get()));            break;
  case tag::regex_result:     call_dtor(static_cast<regex_result&>(*obj.get()));     break;
  case tag::string:           call_dtor(static_cast<string&>(*obj.get()));           break;
  case tag::string_iterator:  call_dtor(static_cast<string_iterator&>(*obj.get()));  break;
  case tag::symbol:           call_dtor(static_cast<value::symbol&>(*obj.get()));    break;
  case tag::type:             call_dtor(static_cast<type&>(*obj.get()));             break;
  case tag::environment:      call_dtor(static_cast<vm::environment&>(*obj.get()));  break;
  default: break;
  }
}

void vv::clear_members(gc::managed_ptr obj)
{
  if (obj.tag() != tag::object)
    g_generic_members.erase(obj);
}

// }}}
// Member and method access {{{

bool vv::has_member(gc::managed_ptr object, const symbol sym)
{
  if (object.tag() == tag::object)
    return value::get<value::object>(object).count(sym);
  const auto mem = g_generic_members.find(object);
  return mem != end(g_generic_members) && mem->second.count(sym);
}

gc::managed_ptr vv::get_member(gc::managed_ptr object, const symbol sym)
{
  if (object.tag() == tag::object)
    return value::get<value::object>(object)[sym];
  return g_generic_members[object][sym];
}

void vv::set_member(gc::managed_ptr object, symbol sym, gc::managed_ptr member)
{
  if (object.tag() == tag::object) {
   value::get<value::object>(object)[sym] = member;
  }
  else {
    g_generic_members[object][sym] = member;
  }
}

void vv::mark_members(gc::managed_ptr object)
{
  if (object.tag() == tag::object) {
    for (auto i : value::get<value::object>(object))
      gc::mark(i.second);
  }
  else {
    const auto mem = g_generic_members.find(object);
    if (mem != end(g_generic_members))
      for (auto i : mem->second)
        gc::mark(i.second);
  }
}

// TODO: optimize this
gc::managed_ptr vv::get_method(gc::managed_ptr type, vv::symbol name)
{
  for (auto i = type; true; i = value::get<value::type>(i).parent) {
    const auto iter = value::get<value::type>(i).methods.find(name);
    if (iter != std::end(value::get<value::type>(i).methods))
      return iter->second;
    if (i == value::get<value::type>(i).parent)
      return {};
  }
}

// }}}

