#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
#include "value/builtin_function.h"
#include "value/integer.h"
#include "value/floating_point.h"
#include "value/string.h"

using namespace vv;
using namespace builtin;
using value::builtin_function;

namespace {

int to_int(dumb_ptr<value::base> boxed)
{
  return static_cast<value::integer&>(*boxed).val;
}

double to_float(dumb_ptr<value::base> boxed)
{
  return static_cast<value::floating_point&>(*boxed).val;
}

// Generic binop generators, if the operator doesn't require any special casing
// and is just a direct wrapper for a C++ equivalent.
template <typename F>
auto fn_int_or_flt_op(const F& op)
{
  return [=](vm::machine& vm) -> value::base*
  {
    vm.self();
    auto left = to_int(vm.top());
    vm.arg(0);
    auto arg = vm.top();
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
  return [=](vm::machine& vm) -> value::base*
  {
    vm.self();
    auto left = to_int(vm.top());
    vm.arg(0);
    auto arg = vm.top();
    if (arg->type != &type::integer)
      return throw_exception("Right-hand argument is not an Integer");
    auto right = to_int(arg);

    return gc::alloc<value::integer, int>( op(left, right) );
  };
}

template <typename F>
auto fn_integer_monop(const F& op)
{
  return [=](vm::machine& vm)
  {
    vm.self();
    return gc::alloc<value::integer, int>( op(to_int(vm.top())) );
  };
}

template <typename F>
auto fn_int_to_flt_monop(const F& op)
{
  return [=](vm::machine& vm)
  {
    vm.self();
    return gc::alloc<value::floating_point>( op(to_int(vm.top())) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](vm::machine& vm) -> value::base*
  {
    vm.self();
    auto self = vm.top();
    vm.arg(0);
    auto arg = vm.top();
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

value::base* fn_integer_divides(vm::machine& vm)
{
  vm.self();
  auto left = to_int(vm.top());
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type == &type::floating_point) {
    if (to_float(arg) == 0.0)
      return throw_exception("cannot divide by zero");
    return gc::alloc<value::floating_point>( left / to_float(arg) );
  }

  if (arg->type != &type::integer)
    return throw_exception("Right-hand argument is not an Integer");
  if (to_int(arg) == 0)
    return throw_exception("cannot divide by zero");
  return gc::alloc<value::integer>( left / to_int(arg));
}

bool boxed_integer_equal(vm::machine& vm)
{
  vm.self();
  auto self = vm.top();
  vm.arg(0);
  auto arg = vm.top();
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

value::base* fn_integer_equals(vm::machine& vm)
{
  return gc::alloc<value::boolean>( boxed_integer_equal(vm) );
}

value::base* fn_integer_unequal(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !boxed_integer_equal(vm) );
}

value::base* fn_integer_pow(vm::machine& vm)
{
  vm.self();
  auto self = vm.top();
  vm.arg(0);
  auto arg = vm.top();
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

value::base* fn_integer_chr(vm::machine& vm)
{
  vm.self();
  auto self = to_int(vm.top());
  if (self < 0 || self > 255)
    return throw_exception("chr can only be called on integers between 0 to 256");
  return gc::alloc<value::string>( std::string{static_cast<char>(self)} );
}

builtin_function int_add      {fn_int_or_flt_op([](auto a, auto b){ return a + b; }), 1};
builtin_function int_subtract {fn_int_or_flt_op([](auto a, auto b){ return a - b; }), 1};
builtin_function int_times    {fn_int_or_flt_op([](auto a, auto b){ return a * b; }), 1};
builtin_function int_divides  {fn_integer_divides,                                    1};
builtin_function int_modulo   {fn_integer_op(std::modulus<int>{}),                    1};
builtin_function int_pow      {fn_integer_pow,                                        1};
builtin_function int_lshift   {fn_integer_op([](int a, int b) { return a << b; }),    1};
builtin_function int_rshift   {fn_integer_op([](int a, int b) { return a >> b; }),    1};
builtin_function int_bitand   {fn_integer_op(std::bit_and<int>{}),                    1};
builtin_function int_bitor    {fn_integer_op(std::bit_or<int>{}),                     1};
builtin_function int_xor      {fn_integer_op(std::bit_xor<int>{}),                    1};
builtin_function int_eq       {fn_integer_equals,                                     1};
builtin_function int_neq      {fn_integer_unequal,                                    1};
builtin_function int_lt       {fn_int_bool_op([](auto a, auto b){ return a < b;  }),  1};
builtin_function int_gt       {fn_int_bool_op([](auto a, auto b){ return a > b;  }),  1};
builtin_function int_le       {fn_int_bool_op([](auto a, auto b){ return a <= b; }),  1};
builtin_function int_ge       {fn_int_bool_op([](auto a, auto b){ return a >= b; }),  1};
builtin_function int_negative {fn_integer_monop(std::negate<int>{}),                  0};
builtin_function int_negate   {fn_integer_monop(std::bit_not<int>{}),                 0};
builtin_function int_sqrt     {fn_int_to_flt_monop(sqrt),                             0};
builtin_function int_sin      {fn_int_to_flt_monop(sin),                              0};
builtin_function int_cos      {fn_int_to_flt_monop(cos),                              0};
builtin_function int_tan      {fn_int_to_flt_monop(tan),                              0};
builtin_function int_chr      {fn_integer_chr,                                        0};

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
