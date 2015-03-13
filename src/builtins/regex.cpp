#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/regex.h"
#include "value/string.h"

using namespace vv;
using namespace builtin;

namespace {

const std::regex& to_regex(const value::base& val)
{
  return static_cast<const value::regex&>(val).val;
}

const std::string& to_string(const value::base& val)
{
  return static_cast<const value::string&>(val).val;
}

// regex {{{

value::base* fn_regex_init(vm::machine& vm)
{
  vm.self();
  auto& regex = static_cast<value::regex&>(*vm.top());
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type == &type::regex)
    regex.val = to_regex(*arg);
  else if (arg->type == &type::string)
    regex.val = std::regex{to_string(*arg)};
  else
    return throw_exception("RegExes can only be constructed from Strings or other RegExes");
  return &regex;
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

value::builtin_function regex_init {fn_regex_init, 1};
value::opt_binop regex_match_index {fn_regex_match_index};

}

value::type type::regex {gc::alloc<value::regex>, {
  { {"init"},        &regex_init       },
  { {"match_index"}, &regex_match_index},
}, builtin::type::object, {"RegEx"}};
