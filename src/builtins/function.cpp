#include "function.h"

#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/builtin_function.h"
#include "value/function.h"
#include "value/partial_function.h"

using namespace vv;
using namespace builtin;

namespace {

size_t get_argc(gc::managed_ptr fn)
{
  switch (fn.tag()) {
  case tag::opt_monop: return 0;
  case tag::opt_binop: return 1;
  case tag::builtin_function: return value::get<value::builtin_function>(fn).argc;
  case tag::function:         return value::get<value::function>(fn).argc;
  default: break;
  }
  return get_argc(value::get<value::partial_function>(fn).function) -
         value::get<value::partial_function>(fn).provided_args.size();
}

}

gc::managed_ptr function::bind(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (get_argc(self) == 0) {
    return throw_exception("Function takes no arguments to bind to");
  }

  if (self.tag() == tag::partial_function) {
    const auto func = value::get<value::partial_function>(self).function;
    auto args = value::get<value::partial_function>(self).provided_args;
    args.push_back(arg);
    return gc::alloc<value::partial_function>( func, args );
  }

  return gc::alloc<value::partial_function>( self, std::vector<gc::managed_ptr>{arg} );
}