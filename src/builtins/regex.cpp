#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
#include "utils/error.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/regex.h"
#include "value/string.h"

using namespace vv;
using namespace builtin;

namespace {

int to_int(const value::base& val)
{
  return static_cast<const value::integer&>(val).val;
}

const std::regex& to_regex(const value::base& val)
{
  return static_cast<const value::regex&>(val).val;
}

const std::string& to_string(const value::base& val)
{
  return static_cast<const value::string&>(val).val;
}

std::pair<const std::smatch&, size_t> get_match_idx(value::base* self, value::base* arg)
{
  if (arg->type != &type::integer) {
    auto str = gc::alloc<value::string>("Index must be an Integer");
    throw vm_error{str};
  }

  auto idx = static_cast<size_t>(to_int(*arg));
  auto& match = static_cast<value::regex_result&>(*self).val;

  if (idx >= match.size()) {
    auto str = gc::alloc<value::string>("Out of range (expected 0-" +
                                        std::to_string(match.size())
                                        += ", got " + std::to_string(idx)
                                        += ')');
    throw vm_error{str};
  }

  return {match, idx};
}

// regex {{{

value::base* fn_regex_init(vm::machine& vm)
{
  vm.self();
  auto& regex = static_cast<value::regex&>(*vm.top());
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type == &type::regex) {
    regex.val = to_regex(*arg);
    regex.str = static_cast<value::regex*>(arg)->str;
  }
  else if (arg->type == &type::string) {
    vm.pre(to_string(*arg));
    regex = to_regex(*vm.top());
  }
  else {
    return throw_exception("RegExes can only be constructed from Strings or other RegExes");
  }
  return &regex;
}

value::base* fn_regex_match(value::base* self, value::base* arg)
{
  if (arg->type != &type::string)
    return throw_exception("RegExes can only be matched against Strings");

  const auto& regex = to_regex(*self);
  const auto& str = to_string(*arg);

  std::smatch results;
  regex_search(str, results, regex);
  return gc::alloc<value::regex_result>( static_cast<value::string&>(*arg),
                                         std::move(results) );
}

value::base* fn_regex_match_index(value::base* self, value::base* arg)
{
  if (arg->type != &type::string)
    return throw_exception("RegExes can only be matched against Strings");

  const auto& regex = to_regex(*self);
  const auto& str = to_string(*arg);

  std::smatch results;
  auto matched = regex_search(str, results, regex);
  if (!matched)
    return gc::alloc<value::nil>( );
  return gc::alloc<value::integer>( static_cast<int>(results.position()) );
}

// }}}
// regex_result {{{

value::base* fn_regex_result_at(value::base* self, value::base* arg)
{

  auto res = get_match_idx(self, arg);
  return gc::alloc<value::string>( res.first[res.second].str() );
}

value::base* fn_regex_result_index(value::base* self, value::base* arg)
{
  auto res = get_match_idx(self, arg);
  return gc::alloc<value::integer>( static_cast<int>(res.first.position(res.second)) );
}

value::base* fn_regex_result_size(value::base* self)
{
  auto sz = static_cast<value::regex_result*>(self)->val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

// }}}

value::builtin_function regex_init{fn_regex_init, 1};
value::opt_binop regex_match      {fn_regex_match};
value::opt_binop regex_match_index{fn_regex_match_index};

value::opt_binop regex_result_at   {fn_regex_result_at};
value::opt_binop regex_result_index{fn_regex_result_index};
value::opt_monop regex_result_size {fn_regex_result_size};

}

value::type type::regex {gc::alloc<value::regex>, {
  { {"init"},        &regex_init       },
  { {"match"},       &regex_match      },
  { {"match_index"}, &regex_match_index}
}, builtin::type::object, {"RegEx"}};

value::type type::regex_result {[]{ return nullptr; }, {
  { {"at"},    &regex_result_at    },
  { {"index"}, &regex_result_index },
  { {"size"},  &regex_result_size  }
}, builtin::type::object, {"RegExResult"}};
