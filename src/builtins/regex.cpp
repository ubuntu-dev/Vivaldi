#include "builtins/regex.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "utils/error.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

std::pair<const std::smatch&, size_t> get_match_idx(gc::managed_ptr self,
                                                    gc::managed_ptr arg)
{
  if (arg.tag() != tag::integer) {
    auto str = gc::alloc<value::string>(message::at_type_error(type::regex_result,
                                        type::integer));
    throw vm_error{str};
  }

  auto idx = static_cast<size_t>(value::get<value::integer>(arg));
  auto& match = value::get<value::regex_result>(self).val;

  if (idx >= match.size()) {
    auto str = gc::alloc<value::string>(message::out_of_range(0, match.size(), idx));
    throw vm_error{str};
  }

  return {match, idx};
}

// regex

gc::managed_ptr regex::init(vm::machine& vm)
{
  vm.self();
  auto regex = vm.top();
  vm.arg(0);
  auto arg = vm.top();
  if (arg.tag() == tag::regex) {
    value::get<value::regex>(regex) = value::get<value::regex>(arg);
  }
  else if (arg.tag() == tag::string) {
    vm.pre(value::get<value::string>(arg));
    value::get<value::regex>(regex) = value::get<value::regex>(vm.top());
  }
  else {
    return throw_exception("RegExes can only be constructed from Strings or other RegExes");
  }
  return regex;
}

gc::managed_ptr regex::match(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::string)
    return throw_exception("RegExes can only be matched against Strings");

  const auto& regex = value::get<value::regex>(self).val;
  const auto& str = value::get<value::string>(arg);

  std::smatch results;
  regex_search(str, results, regex);

  return gc::alloc<value::regex_result>( arg, std::move(results) );
}

gc::managed_ptr regex::match_index(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::string)
    return throw_exception("RegExes can only be matched against Strings");

  const auto& regex = value::get<value::regex>(self).val;
  const auto& str = value::get<value::string>(arg);

  std::smatch results;
  auto matched = regex_search(str, results, regex);
  if (!matched)
    return gc::alloc<value::nil>( );
  return gc::alloc<value::integer>( static_cast<int>(results.position()) );
}

// regex_result

gc::managed_ptr regex_result::at(gc::managed_ptr self, gc::managed_ptr arg)
{

  auto res = get_match_idx(self, arg);
  return gc::alloc<value::string>( res.first[res.second].str() );
}

gc::managed_ptr regex_result::index(gc::managed_ptr self, gc::managed_ptr arg)
{
  auto res = get_match_idx(self, arg);
  return gc::alloc<value::integer>( static_cast<int>(res.first.position(res.second)) );
}

gc::managed_ptr regex_result::size(gc::managed_ptr self)
{
  auto sz = value::get<value::regex_result>(self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}
