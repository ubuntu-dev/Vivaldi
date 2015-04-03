#include "builtins.h"

#include "gc/alloc.h"
#include "messages.h"
#include "utils/lang.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/floating_point.h"
#include "value/integer.h"
#include "value/opt_functions.h"
#include "value/string.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;
using value::builtin_function;
using value::opt_monop;
using value::opt_binop;

namespace {

int to_int(value::basic_object* boxed)
{
  return static_cast<value::integer&>(*boxed).val;
}

double to_float(value::basic_object* boxed)
{
  return static_cast<value::floating_point&>(*boxed).val;
}

// Generic binop generators, if the operator doesn't require any special casing
// and is just a direct wrapper for a C++ equivalent.
template <typename F>
auto fn_int_or_flt_op(const F& op)
{
  return [=](value::basic_object* self, value::basic_object* arg) -> value::basic_object*
  {
    auto left = to_int(self);
    if (arg->type == &type::floating_point)
      return gc::alloc<value::floating_point>( op(left, to_float(arg)) );

    if (arg->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer");
    return gc::alloc<value::integer, int>( op(left, to_int(arg)) );
  };
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](value::basic_object* self, value::basic_object* arg) -> value::basic_object*
  {
    auto left = to_int(self);
    if (arg->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer");
    auto right = to_int(arg);

    return gc::alloc<value::integer, int>( op(left, right) );
  };
}

template <typename F>
auto fn_integer_monop(const F& op)
{
  return [=](value::basic_object* self)
  {
    return gc::alloc<value::integer, int>( op(to_int(self)) );
  };
}

template <typename F>
auto fn_int_to_flt_monop(const F& op)
{
  return [=](value::basic_object* self)
  {
    return gc::alloc<value::floating_point>( op(to_int(self)) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](value::basic_object* self, value::basic_object* arg) -> value::basic_object*
  {
    if (arg->type == &type::floating_point) {
      auto left = to_int(self);
      auto right = to_float(arg);
      return gc::alloc<value::boolean>( op(left, right) );
    }
    if (arg->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer");

    auto left = to_int(self);
    auto right = to_int(arg);
    return gc::alloc<value::boolean>( op(left, right) );
  };
}

value::basic_object* fn_integer_divides(value::basic_object* self, value::basic_object* arg)
{
  auto left = to_int(self);
  if (arg->type == &type::floating_point) {
    if (to_float(arg) == 0.0)
      return throw_exception(message::divide_by_zero);
    return gc::alloc<value::floating_point>( left / to_float(arg) );
  }

  if (arg->type != &type::integer)
    return throw_exception("Right-hand argument is not an Integer");
  if (to_int(arg) == 0)
    return throw_exception(message::divide_by_zero);
  return gc::alloc<value::integer>( left / to_int(arg));
}

bool boxed_integer_equal(value::basic_object* self, value::basic_object* arg)
{
  if (arg->type == &type::floating_point) {
    auto left = to_int(self);
    auto right = to_float(arg);
    return left == right;
  }
  if (arg->type != &type::integer)
    return false;

  auto left = to_int(self);
  auto right = to_int(arg);
  return left == right;
}

value::basic_object* fn_integer_equals(value::basic_object* left, value::basic_object* right)
{
  return gc::alloc<value::boolean>( boxed_integer_equal(left, right) );
}

value::basic_object* fn_integer_unequal(value::basic_object* left, value::basic_object* right)
{
  return gc::alloc<value::boolean>( !boxed_integer_equal(left, right) );
}

value::basic_object* fn_integer_pow(value::basic_object* self, value::basic_object* arg)
{
  if (arg->type == &type::floating_point) {
    auto left = to_int(self);
    auto right = to_float(arg);
    return gc::alloc<value::floating_point>( pow(left, right) );
  }

  auto left = to_int(self);
  if (arg->type != &type::integer)
    return throw_exception("Right-hand argument is not an Integer");
  auto right = to_int(arg);

  if (right < 0)
    return gc::alloc<value::floating_point>( pow(left, right) );
  return gc::alloc<value::integer>( static_cast<int>(pow(left, right)) );
}

value::basic_object* fn_integer_chr(value::basic_object* self)
{
  auto ord = to_int(self);
  if (ord < 0 || ord > 255)
    return throw_exception(message::out_of_range(0, 256, ord));
  return gc::alloc<value::string>( std::string{static_cast<char>(ord)} );
}

opt_binop int_add      {fn_int_or_flt_op([](auto a, auto b){ return a + b; })};
opt_binop int_subtract {fn_int_or_flt_op([](auto a, auto b){ return a - b; })};
opt_binop int_times    {fn_int_or_flt_op([](auto a, auto b){ return a * b; })};
opt_binop int_divides  {fn_integer_divides                                   };
opt_binop int_modulo   {fn_integer_op(std::modulus<int>{})                   };
opt_binop int_pow      {fn_integer_pow                                       };
opt_binop int_lshift   {fn_integer_op([](int a, int b) { return a << b; })};
opt_binop int_rshift   {fn_integer_op([](int a, int b) { return a >> b; })};
opt_binop int_bitand   {fn_integer_op(std::bit_and<int>{}),               };
opt_binop int_bitor    {fn_integer_op(std::bit_or<int>{}),                };
opt_binop int_xor      {fn_integer_op(std::bit_xor<int>{}),               };
opt_binop int_eq       {fn_integer_equals };
opt_binop int_neq      {fn_integer_unequal};
opt_binop int_lt       {fn_int_bool_op([](auto a, auto b){ return a < b;  })};
opt_binop int_gt       {fn_int_bool_op([](auto a, auto b){ return a > b;  })};
opt_binop int_le       {fn_int_bool_op([](auto a, auto b){ return a <= b; })};
opt_binop int_ge       {fn_int_bool_op([](auto a, auto b){ return a >= b; })};
opt_monop int_negative {fn_integer_monop(std::negate<int>{})                };
opt_monop int_negate   {fn_integer_monop(std::bit_not<int>{})               };
opt_monop int_sqrt     {fn_int_to_flt_monop(sqrt)                           };
opt_monop int_sin      {fn_int_to_flt_monop(sin)                            };
opt_monop int_cos      {fn_int_to_flt_monop(cos)                            };
opt_monop int_tan      {fn_int_to_flt_monop(tan)                            };
opt_monop int_chr      {fn_integer_chr                                      };

}

value::type type::integer{[]{ return nullptr; }, {
  { {"add"},            &int_add      },
  { {"subtract"},       &int_subtract },
  { {"times"},          &int_times    },
  { {"divides"},        &int_divides  },
  { {"modulo"},         &int_modulo   },
  { {"pow"},            &int_pow      },
  { {"bitand"},         &int_bitand   },
  { {"bitor"},          &int_bitor    },
  { {"xor"},            &int_xor      },
  { {"lshift"},         &int_lshift   },
  { {"rshift"},         &int_rshift   },
  { {"equals"},         &int_eq       },
  { {"unequal"},        &int_neq      },
  { {"less"},           &int_lt       },
  { {"greater"},        &int_gt       },
  { {"less_equals"},    &int_le       },
  { {"greater_equals"}, &int_ge       },
  { {"negative"},       &int_negative },
  { {"negate"},         &int_negate   },
  { {"sqrt"},           &int_sqrt     },
  { {"sin"},            &int_sin      },
  { {"cos"},            &int_cos      },
  { {"tan"},            &int_tan      },
  { {"chr"},            &int_chr      }
}, builtin::type::object, {"Integer"}};
