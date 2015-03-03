#include "builtins.h"

#include "utils/lang.h"
#include "gc.h"
#include "value/opt_functions.h"
#include "value/floating_point.h"
#include "value/integer.h"

using namespace vv;
using namespace builtin;
using value::opt_monop;
using value::opt_binop;

namespace {

bool is_float(dumb_ptr<value::base> boxed) noexcept
{
  return boxed->type == &type::floating_point || boxed->type == &type::integer;
}

double to_float(dumb_ptr<value::base> boxed) noexcept
{
  if (boxed->type == &type::floating_point)
    return static_cast<const value::floating_point&>(*boxed).val;
  return static_cast<double>(static_cast<value::integer&>(*boxed).val);
}

template <typename F>
auto fn_floating_point_op(const F& op)
{
  return [=](value::base* self, value::base* arg)
  {
    if (!is_float(arg))
      return throw_exception("Right-hand argument is not a Float");
    auto res = gc::alloc<value::floating_point>(op(to_float(self), to_float(arg)));
    return static_cast<value::base*>(res);
  };
}

template <typename F>
auto fn_float_bool_op(const F& op)
{
  return [=](value::base* self, value::base* arg)
  {
    if (!is_float(arg))
      return throw_exception("Right-hand argument is not a Float");
    auto res = gc::alloc<value::boolean>( op(to_float(self), to_float(arg)) );
    return static_cast<value::base*>(res);
  };
}

template <typename F>
auto fn_floating_point_monop(const F& op)
{
  return [=](value::base* self)
  {
    return gc::alloc<value::floating_point>( op(to_float(self)) );
  };
}

value::base* fn_floating_point_divides(value::base* self, value::base* arg)
{
  if (!is_float(arg))
    return throw_exception("Right-hand argument is not a Float");
  if (to_float(arg) == 0)
    return throw_exception("Cannot divide by zero");
  return gc::alloc<value::floating_point>( to_float(self) / to_float(arg) );
}

opt_binop flt_add      {fn_floating_point_op(std::plus<double>{})      };
opt_binop flt_subtract {fn_floating_point_op(std::minus<double>{})     };
opt_binop flt_times    {fn_floating_point_op(std::multiplies<double>{})};
opt_binop flt_divides  {fn_floating_point_divides                      };
opt_binop flt_pow      {fn_floating_point_op(pow)                      };
opt_binop flt_eq       {fn_float_bool_op(std::equal_to<double>{})      };
opt_binop flt_neq      {fn_float_bool_op(std::not_equal_to<double>{})  };
opt_binop flt_lt       {fn_float_bool_op(std::less<double>{})          };
opt_binop flt_gt       {fn_float_bool_op(std::greater<double>{})       };
opt_binop flt_le       {fn_float_bool_op(std::less_equal<double>{})    };
opt_binop flt_ge       {fn_float_bool_op(std::greater_equal<double>{}) };
opt_monop flt_negative {fn_floating_point_monop(std::negate<double>{}) };
opt_monop flt_sqrt     {fn_floating_point_monop(sqrt)                  };
opt_monop flt_sin      {fn_floating_point_monop(sin)                   };
opt_monop flt_cos      {fn_floating_point_monop(cos)                   };
opt_monop flt_tan      {fn_floating_point_monop(tan)                   };
}
value::type type::floating_point{[]{ return nullptr; }, {
  { {"add"},            &flt_add      },
  { {"subtract"},       &flt_subtract },
  { {"times"},          &flt_times    },
  { {"divides"},        &flt_divides  },
  { {"pow"},            &flt_pow      },
  { {"equals"},         &flt_eq       },
  { {"unequal"},        &flt_neq      },
  { {"less"},           &flt_lt       },
  { {"greater"},        &flt_gt       },
  { {"less_equals"},    &flt_le       },
  { {"greater_equals"}, &flt_ge       },
  { {"negative"},       &flt_negative },
  { {"sqrt"},           &flt_sqrt     },
  { {"sin"},            &flt_sin      },
  { {"cos"},            &flt_cos      },
  { {"tan"},            &flt_tan      }
}, builtin::type::object, {"Float"}};
