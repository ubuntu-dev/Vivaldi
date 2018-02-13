#include "builtins/string.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "utils/string_helpers.h"
#include "value/floating_point.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"

using namespace vv;
using namespace builtin;

// Generic helper functions {{{

namespace {

template <typename F>
auto fn_string_cmp(const F& cmp)
{
  return [cmp](gc::managed_ptr self, gc::managed_ptr arg) -> gc::managed_ptr
  {
    if (arg.tag() != tag::string)
      return throw_exception(type::type_error,
                             "Strings can only be compared to other Strings");
    return gc::alloc<value::boolean>( cmp(value::get<value::string>(self),
                                          value::get<value::string>(arg)) );
  };
}

}

// }}}

// string

gc::managed_ptr string::init(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() == tag::string)
    value::get<value::string>(self) = value::get<value::string>(arg);
  else if (arg.tag() == tag::symbol)
    value::get<value::string>(self) = to_string(value::get<value::symbol>(arg));
  else
     value::get<value::string>(self) = value_for(arg);
  return self;
}

gc::managed_ptr string::size(gc::managed_ptr self)
{
  const auto sz = value::get<value::string>(self).size();
  return gc::alloc<value::integer>( static_cast<value::integer>(sz) );
}

gc::managed_ptr string::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::string)
    return gc::alloc<value::boolean>( false );

  return gc::alloc<value::boolean>( value::get<value::string>(self) ==
                                    value::get<value::string>(arg) );
}

gc::managed_ptr string::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::string)
    return gc::alloc<value::boolean>( true );

  return gc::alloc<value::boolean>( value::get<value::string>(self) !=
                                    value::get<value::string>(arg) );
}

gc::managed_ptr string::less(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_string_cmp(std::less<std::string>{})(self, arg);
}

gc::managed_ptr string::greater(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_string_cmp(std::greater<std::string>{})(self, arg);
}

gc::managed_ptr string::less_equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_string_cmp(std::less_equal<std::string>{})(self, arg);
}

gc::managed_ptr string::greater_equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_string_cmp(std::greater_equal<std::string>{})(self, arg);
}

gc::managed_ptr string::add(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() == tag::string) {
    const auto str = value::get<value::string>(arg);
    return gc::alloc<value::string>( value::get<value::string>(self) + str );
  }

  if (arg.tag() == tag::character) {
    const auto chr = value::get<value::character>(arg);
    return gc::alloc<value::string>( value::get<value::string>(self) + chr );
  }

  return throw_exception(type::type_error,
                         message::add_type_error(type::string, type::string));
}

gc::managed_ptr string::times(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::integer)
    return throw_exception(type::type_error,
                           "Strings can only be multiplied by Integers");

  const auto& val = value::get<value::string>(self);
  std::string new_str{};
  for (auto i = value::get<value::integer>(arg); i--;)
    new_str += val;
  return gc::alloc<value::string>( new_str );
}

gc::managed_ptr string::to_int(gc::managed_ptr self)
{
  return gc::alloc<value::integer>(vv::to_int(value::get<value::string>(self)));
}

gc::managed_ptr string::to_flt(gc::managed_ptr self)
{
  return gc::alloc<value::floating_point>(std::stof(value::get<value::string>(self)));
}

gc::managed_ptr string::to_sym(gc::managed_ptr self)
{
  return gc::alloc<value::symbol>(std::string_view{value::get<value::string>(self)});
}

gc::managed_ptr string::at(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::integer)
    return throw_exception(type::range_error,
                           message::at_type_error(type::string, type::integer));

  const auto val = value::get<value::integer>(arg);
  const auto& str = value::get<value::string>(self);

  if (str.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception(type::range_error,
                           message::out_of_range(0, str.size(), val));

  return gc::alloc<value::character>( str[static_cast<unsigned>(val)] );
}

gc::managed_ptr string::start(gc::managed_ptr self)
{
  return gc::alloc<value::string_iterator>(self);
}

gc::managed_ptr string::stop(gc::managed_ptr self)
{
  const auto end = gc::alloc<value::string_iterator>(self);
  value::get<value::string_iterator>(end).idx = value::get<value::string>(self).size();
  return end;
}

gc::managed_ptr string::to_upper(gc::managed_ptr self)
{
  auto str = value::get<value::string>(self);
  transform(begin(str), end(str), begin(str), toupper);
  return gc::alloc<value::string>( str );
}

gc::managed_ptr string::to_lower(gc::managed_ptr self)
{
  auto str = value::get<value::string>(self);
  transform(begin(str), end(str), begin(str), tolower);
  return gc::alloc<value::string>( str );
}

gc::managed_ptr string::starts_with(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::string)
    return throw_exception(type::type_error,
                           "Strings can only start with other Strings");

  const auto& str = value::get<value::string>(self);
  const auto& other = value::get<value::string>(arg);

  if (other.size() > str.size() || !equal(begin(other), end(other), begin(str)))
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( true );
}

gc::managed_ptr string::ord(gc::managed_ptr self)
{
  const auto& str = value::get<value::string>(self);
  if (str.empty())
    return throw_exception(type::range_error,
                           "Cannot call ord on an empty string");
  return gc::alloc<value::integer, value::integer>( str[0] );
}

gc::managed_ptr string::split(vm::machine& vm)
{
  vm.self();
  boost::string_ref str{value::get<value::string>(vm.top())};
  vm.arg(0);
  if (vm.top().tag() != tag::string)
    return throw_exception(type::type_error,
                           "Strings can only be split by other Strings");
  const auto& sep = value::get<value::string>(vm.top());

  size_t substrs{};

  for (;;) {
    ++substrs;
    if (str.empty()) {
      vm.pstr("");
      break;
    }
    const auto next_sep = str.find(sep);
    if (next_sep == boost::string_ref::npos) {
      vm.pstr({begin(str), end(str)});
      break;
    }
    const auto substr = str.substr(0, next_sep);
    vm.pstr({begin(substr), end(substr)});
    str = str.substr(next_sep + sep.size());
  }
  vm.parr(substrs);
  return vm.top();
}

gc::managed_ptr string::replace(vm::machine& vm)
{
  vm.arg(1);
  if (vm.top().tag() != tag::string)
    return throw_exception(type::type_error,
                           "Replacements must be other Strings");
  const auto& replacement = value::get<value::string>(vm.top());

  vm.arg(0);
  if (vm.top().tag() != tag::regex)
    return throw_exception(type::type_error,
                           "Strings can only be replaced by RegExes");
  const auto& re = value::get<value::regex>(vm.top()).val;

  vm.self();
  const auto& str = value::get<value::string>(vm.top());

  vm.pop(3);

  return gc::alloc<value::string>( regex_replace(str, re, replacement) );
}

// string_iterator

gc::managed_ptr string_iterator::at_start(gc::managed_ptr self)
{
  const auto& iter = value::get<value::string_iterator>(self);
  return gc::alloc<value::boolean>( iter.idx == 0 );
}

gc::managed_ptr string_iterator::at_end(gc::managed_ptr self)
{
  const auto& iter = value::get<value::string_iterator>(self);
  return gc::alloc<value::boolean>(iter.idx ==
                                   value::get<value::string>(iter.str).size());
}

gc::managed_ptr string_iterator::get(gc::managed_ptr self)
{
  const auto& iter = value::get<value::string_iterator>(self);
  if (iter.idx == value::get<value::string>(iter.str).size())
    return throw_exception(type::range_error,
                           message::iterator_at_end(type::string_iterator));

  const auto chr = value::get<value::string>(iter.str)[iter.idx];
  return gc::alloc<value::character>( chr );
}

gc::managed_ptr string_iterator::increment(gc::managed_ptr self)
{
  auto& iter = value::get<value::string_iterator>(self);
  if (iter.idx == value::get<value::string>(iter.str).size())
    return throw_exception(type::range_error,
                           message::iterator_past_end(type::string_iterator));

  ++iter.idx;
  return self;
}

gc::managed_ptr string_iterator::decrement(gc::managed_ptr self)
{
  auto& iter = value::get<value::string_iterator>(self);
  if (iter.idx == 0)
    return throw_exception(type::range_error,
                           message::iterator_past_start(type::string_iterator));

  --iter.idx;
  return self;
}

gc::managed_ptr string_iterator::add(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::string_iterator>(self);

  if (arg.tag() != tag::integer) {
    return throw_exception(type::type_error,
                           message::add_type_error(type::string_iterator,
                                                   type::integer));
  }

  const auto offset = value::get<value::integer>(arg);

  if (static_cast<int>(iter.idx) + offset < 0)
    return throw_exception(type::range_error,
                           message::iterator_past_start(type::string_iterator));
  if (iter.idx + offset > value::get<value::string>(iter.str).size())
    return throw_exception(type::range_error,
                           message::iterator_past_end(type::string_iterator));

  const auto other = gc::alloc<value::string_iterator>( iter.str );
  value::get<value::string_iterator>(other).idx = iter.idx + offset;
  return other;
}

gc::managed_ptr string_iterator::subtract(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::string_iterator>(self);

  if (arg.tag() != tag::integer)
    return throw_exception(type::type_error,
                           "Only Integers can be subtracted from StringIterators");
  const auto offset = value::get<value::integer>(arg);

  if (static_cast<int>(iter.idx) - offset < 0)
    return throw_exception(type::range_error,
                           message::iterator_past_start(type::string_iterator));
  const auto str_sz = value::get<value::string>(iter.str).size();
  if (static_cast<int>(iter.idx) - offset > static_cast<int>(str_sz))
    return throw_exception(type::range_error,
                           message::iterator_past_end(type::string_iterator));

  const auto other = gc::alloc<value::string_iterator>( iter.str );
  value::get<value::string_iterator>(other).idx = iter.idx - offset;
  return other;
}

gc::managed_ptr string_iterator::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::string_iterator>(self);
  const auto& other = value::get<value::string_iterator>(arg);
  return gc::alloc<value::boolean>(iter.str == other.str && iter.idx == other.idx);
}

gc::managed_ptr string_iterator::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  auto& iter = value::get<value::string_iterator>(self);
  auto& other = value::get<value::string_iterator>(arg);
  return gc::alloc<value::boolean>(iter.str != other.str || iter.idx != other.idx);
}
