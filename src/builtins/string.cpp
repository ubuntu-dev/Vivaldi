#include "builtins.h"

#include "gc/alloc.h"
#include "messages.h"
#include "utils/lang.h"
#include "utils/string_helpers.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

namespace {

int to_int(value::object* boxed)
{
  return static_cast<value::integer&>(*boxed).val;
}

const std::string& to_string(value::object* boxed)
{
  return static_cast<const value::string&>(*boxed).val;
}

vv::symbol to_symbol(value::object* boxed)
{
  return static_cast<const value::symbol&>(*boxed).val;
}

// string {{{

value::object* fn_string_init(vm::machine& vm)
{
  vm.self();
  auto str = static_cast<value::string*>(vm.top());
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type == &type::string)
    str->val = to_string(arg);
  else if (arg->type == &type::symbol)
    str->val = to_string(to_symbol(arg));
  else
     str->val = value_for(*arg);
  return str;
}

value::object* fn_string_size(value::object* self)
{
  auto sz = static_cast<value::string&>(*self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::object* fn_string_equals(value::object* self, value::object* arg)
{
  if (arg->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( to_string(self) == to_string(arg) );
}

value::object* fn_string_unequal(value::object* self, value::object* arg)
{
  if (arg->type != &type::string)
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( to_string(self) != to_string(arg) );
}

template <typename F>
auto fn_string_cmp(const F& cmp)
{
  return [cmp](value::object* self, value::object* arg) -> value::object*
  {
    if (arg->type != &type::string)
      return throw_exception("Strings can only be compared to other Strings");
    return gc::alloc<value::boolean>( cmp(to_string(self), to_string(arg)) );
  };
}

value::object* fn_string_add(value::object* self, value::object* arg)
{
  if (arg->type != &type::string)
    return throw_exception(message::add_type_error(type::string, type::string));

  auto str = to_string(arg);
  auto new_str = static_cast<value::string&>(*self).val + str;
  return gc::alloc<value::string>( new_str );
}

value::object* fn_string_times(value::object* self, value::object* arg)
{
  if (arg->type != &type::integer)
    return throw_exception("Strings can only be multiplied by Integers");

  auto val = static_cast<value::string&>(*self).val;
  std::string new_str{};
  for (auto i = to_int(self); i--;)
    new_str += val;
  return gc::alloc<value::string>( new_str );
}

value::object* fn_string_to_int(value::object* self)
{
  return gc::alloc<value::integer>(vv::to_int(to_string(self)));
}

value::object* fn_string_at(value::object* self, value::object* arg)
{
  if (arg->type != &type::integer)
    return throw_exception(message::at_type_error(type::string, type::integer));
  auto val = static_cast<value::integer&>(*arg).val;
  const auto& str = static_cast<value::string&>(*self).val;
  if (str.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception(message::out_of_range(0, str.size(), val));
  return gc::alloc<value::string>( std::string{str[static_cast<unsigned>(val)]} );
}

value::object* fn_string_start(value::object* self)
{
  auto str = static_cast<value::string*>(self);
  return gc::alloc<value::string_iterator>(*str);
}

value::object* fn_string_stop(value::object* self)
{
  auto str = static_cast<value::string*>(self);
  auto end = gc::alloc<value::string_iterator>(*str);
  static_cast<value::string_iterator&>(*end).idx = str->val.size();
  return end;
}

value::object* fn_string_to_upper(value::object* self)
{
  auto str = static_cast<value::string&>(*self).val;
  transform(begin(str), end(str), begin(str), toupper);
  return gc::alloc<value::string>( str );
}

value::object* fn_string_to_lower(value::object* self)
{
  auto str = static_cast<value::string&>(*self).val;
  transform(begin(str), end(str), begin(str), tolower);
  return gc::alloc<value::string>( str );
}

value::object* fn_string_starts_with(value::object* self, value::object* arg)
{
  if (arg->type != &type::string)
    return throw_exception("Strings can only start with other Strings");

  const auto& str = static_cast<value::string&>(*self).val;
  const auto& other = to_string(arg);

  if (other.size() > str.size() || !equal(begin(other), end(other), begin(str)))
    return gc::alloc<value::boolean>( false );
  return gc::alloc<value::boolean>( true );
}

value::object* fn_string_ord(value::object* self)
{
  auto str = static_cast<value::string&>(*self).val;
  if (!str.size())
    return throw_exception("Cannot call ord on an empty string");
  return gc::alloc<value::integer, int>( str[0] );
}

value::object* fn_string_split(vm::machine& vm)
{
  vm.self();
  boost::string_ref str{static_cast<value::string&>(*vm.top()).val};
  vm.arg(0);
  if (vm.top()->type != &type::string)
    return throw_exception("Strings can only be split by other Strings");
  const auto& sep = static_cast<value::string&>(*vm.top()).val;

  size_t substrs{};

  for (;;) {
    ++substrs;
    if (!str.size()) {
      vm.pstr("");
      break;
    }
    auto next_sep = str.find(sep);
    if (next_sep == boost::string_ref::npos) {
      vm.pstr({begin(str), end(str)});
      break;
    }
    auto substr = str.substr(0, next_sep);
    vm.pstr({begin(substr), end(substr)});
    str = str.substr(next_sep + sep.size());
  }
  vm.parr(substrs);
  return vm.top();
}

value::object* fn_string_replace(vm::machine& vm)
{
  vm.arg(1);
  if (vm.top()->type != &type::string)
    return throw_exception("Replacements must be other Strings");
  const auto& replacement = to_string(vm.top());

  vm.arg(0);
  if (vm.top()->type != &type::regex)
    return throw_exception("Strings can only be replaced by RegExes");
  const auto& re = static_cast<value::regex&>(*vm.top()).val;

  vm.self();
  const auto& str = to_string(vm.top());

  vm.pop(3);

  return gc::alloc<value::string>( regex_replace(str, re, replacement) );
}

// }}}
// string_iterator {{{

value::object* fn_string_iterator_at_start(value::object* self)
{
  auto& iter = static_cast<value::string_iterator&>(*self);
  return gc::alloc<value::boolean>( iter.idx == 0 );
}

value::object* fn_string_iterator_at_end(value::object* self)
{
  auto& iter = static_cast<value::string_iterator&>(*self);
  return gc::alloc<value::boolean>( iter.idx == iter.str.val.size() );
}

value::object* fn_string_iterator_get(value::object* self)
{
  auto& iter = static_cast<value::string_iterator&>(*self);
  if (iter.idx == iter.str.val.size())
    return throw_exception(message::iterator_at_end(type::string_iterator));
  return gc::alloc<value::string>( std::string{iter.str.val[iter.idx]} );
}

value::object* fn_string_iterator_increment(value::object* self)
{
  auto iter = static_cast<value::string_iterator*>(self);
  if (iter->idx == iter->str.val.size())
    return throw_exception(message::iterator_past_end(type::string_iterator));
  iter->idx += 1;
  return iter;
}

value::object* fn_string_iterator_decrement(value::object* self)
{
  auto iter = static_cast<value::string_iterator*>(self);
  if (iter->idx == 0)
    return throw_exception(message::iterator_past_start(type::string_iterator));
  iter->idx -= 1;
  return iter;
}

value::object* fn_string_iterator_add(value::object* self, value::object* arg)
{
  auto& iter = static_cast<value::string_iterator&>(*self);

  if (arg->type != &type::integer)
    return throw_exception(message::add_type_error(type::string_iterator,
                                                   type::integer));
  auto offset = to_int(arg);

  if (static_cast<int>(iter.idx) + offset < 0)
    return throw_exception(message::iterator_past_start(type::string_iterator));
  if (iter.idx + offset > iter.str.val.size())
    return throw_exception(message::iterator_past_end(type::string_iterator));

  auto other = gc::alloc<value::string_iterator>( iter );
  static_cast<value::string_iterator&>(*other).idx = iter.idx + offset;
  return other;
}

value::object* fn_string_iterator_subtract(value::object* self, value::object* arg)
{
  auto& iter = static_cast<value::string_iterator&>(*self);

  if (arg->type != &type::integer)
    return throw_exception("Only Integers can be subtracted from StringIterators");
  auto offset = to_int(arg);

  if (static_cast<int>(iter.idx) - offset < 0)
    return throw_exception(message::iterator_past_start(type::string_iterator));
  if (static_cast<int>(iter.idx) - offset > static_cast<int>(iter.str.val.size()))
    return throw_exception(message::iterator_past_end(type::string_iterator));

  auto other = gc::alloc<value::string_iterator>( iter );
  static_cast<value::string_iterator&>(*other).idx = iter.idx - offset;
  return other;
}

value::object* fn_string_iterator_equals(value::object* self, value::object* arg)
{
  auto& iter = static_cast<value::string_iterator&>(*self);
  auto& other = static_cast<value::string_iterator&>(*arg);
  return gc::alloc<value::boolean>(&iter.str == &other.str && iter.idx == other.idx);
}

value::object* fn_string_iterator_unequal(value::object* self, value::object* arg)
{
  auto& iter = static_cast<value::string_iterator&>(*self);
  auto& other = static_cast<value::string_iterator&>(*arg);
  return gc::alloc<value::boolean>(&iter.str != &other.str || iter.idx != other.idx);
}

// }}}

value::builtin_function string_init {fn_string_init, 1};
value::opt_monop string_size        {fn_string_size   };

value::opt_binop string_equals  {fn_string_equals                                };
value::opt_binop string_unequal {fn_string_unequal                               };
value::opt_binop string_greater {fn_string_cmp(std::greater<std::string>{})      };
value::opt_binop string_less    {fn_string_cmp(std::less<std::string>{})         };
value::opt_binop string_ge      {fn_string_cmp(std::greater_equal<std::string>{})};
value::opt_binop string_le      {fn_string_cmp(std::less_equal<std::string>{})   };

value::opt_binop string_add            {fn_string_add        };
value::opt_binop string_times          {fn_string_times      };
value::opt_monop string_to_int         {fn_string_to_int     };
value::opt_binop string_at             {fn_string_at         };
value::opt_monop string_start          {fn_string_start      };
value::opt_monop string_stop           {fn_string_stop       };
value::opt_monop string_to_upper       {fn_string_to_upper   };
value::opt_monop string_to_lower       {fn_string_to_lower   };
value::opt_binop string_starts_with    {fn_string_starts_with};
value::opt_monop string_ord            {fn_string_ord        };
value::builtin_function string_split   {fn_string_split,    1};
value::builtin_function string_replace {fn_string_replace,  2};

value::opt_monop string_iterator_at_start  {fn_string_iterator_at_start };
value::opt_monop string_iterator_at_end    {fn_string_iterator_at_end   };
value::opt_monop string_iterator_get       {fn_string_iterator_get      };
value::opt_binop string_iterator_equals    {fn_string_iterator_equals   };
value::opt_binop string_iterator_unequal   {fn_string_iterator_unequal  };
value::opt_monop string_iterator_increment {fn_string_iterator_increment};
value::opt_monop string_iterator_decrement {fn_string_iterator_decrement};
value::opt_binop string_iterator_add       {fn_string_iterator_add      };
value::opt_binop string_iterator_subtract  {fn_string_iterator_subtract };

}

value::type type::string {gc::alloc<value::string>, {
  { {"init"},           &string_init        },
  { {"size"},           &string_size        },
  { {"equals"},         &string_equals      },
  { {"unequal"},        &string_unequal     },
  { {"greater"},        &string_greater     },
  { {"less"},           &string_less        },
  { {"greater_equals"}, &string_ge          },
  { {"less_equals"},    &string_le          },
  { {"add"},            &string_add         },
  { {"times"},          &string_times       },
  { {"to_int"},         &string_to_int      },
  { {"at"},             &string_at          },
  { {"start"},          &string_start       },
  { {"stop"},           &string_stop        },
  { {"to_upper"},       &string_to_upper    },
  { {"to_lower"},       &string_to_lower    },
  { {"starts_with"},    &string_starts_with },
  { {"ord"},            &string_ord         },
  { {"split"},          &string_split       },
  { {"replace"},        &string_replace     }
}, builtin::type::object, {"String"}};

value::type type::string_iterator {[]{ return nullptr; }, {
  { {"at_start"},  &string_iterator_at_start  },
  { {"at_end"},    &string_iterator_at_end    },
  { {"get"},       &string_iterator_get       },
  { {"equals"},    &string_iterator_equals    },
  { {"unequal"},   &string_iterator_unequal   },
  { {"increment"}, &string_iterator_increment },
  { {"decrement"}, &string_iterator_decrement },
  { {"add"},       &string_iterator_add       },
  { {"subtract"},  &string_iterator_subtract  },
}, builtin::type::object, {"StringIterator"}};
