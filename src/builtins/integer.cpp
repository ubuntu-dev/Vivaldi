#include "builtins/integer.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/floating_point.h"
#include "value/string.h"

using namespace vv;
using namespace builtin;

// Generic binop generators {{{

// Used if the operator doesn't require any special casing and is just a direct
// wrapper for a C++ equivalent.

namespace {

template <typename F>
auto fn_int_or_flt_op(const F& op)
{
  return [=](gc::managed_ptr self, gc::managed_ptr arg) -> gc::managed_ptr
  {
    auto left = value::get<value::integer>(self);
    if (arg.tag() == tag::floating_point)
      return gc::alloc<value::floating_point>( op(left, value::get<value::floating_point>(arg)) );

    if (arg.tag() != tag::integer) {
      return throw_exception(type::type_error,
                             "Right-hand argument is not an Integer");
    }

    return gc::alloc<value::integer, int>( op(left, value::get<value::integer>(arg)) );
  };
}

template <typename F>
auto fn_integer_op(const F& op)
{
  return [=](gc::managed_ptr self, gc::managed_ptr arg) -> gc::managed_ptr
  {
    if (arg.tag() != tag::integer) {
      return throw_exception(type::type_error,
                             "Right-hand argument is not an Integer");
    }

    auto left = value::get<value::integer>(self);
    auto right = value::get<value::integer>(arg);
    return gc::alloc<value::integer, int>( op(left, right) );
  };
}

template <typename F>
auto fn_integer_monop(const F& op)
{
  return [=](gc::managed_ptr self)
  {
    return gc::alloc<value::integer, int>( op(value::get<value::integer>(self)) );
  };
}

template <typename F>
auto fn_int_to_flt_monop(const F& op)
{
  return [=](gc::managed_ptr self)
  {
    return gc::alloc<value::floating_point>( op(value::get<value::integer>(self)) );
  };
}

template <typename F>
auto fn_int_bool_op(const F& op)
{
  return [=](gc::managed_ptr self, gc::managed_ptr arg) -> gc::managed_ptr
  {
    if (arg.tag() == tag::floating_point) {
      auto left = value::get<value::integer>(self);
      auto right = value::get<value::floating_point>(arg);
      return gc::alloc<value::boolean>( op(left, right) );
    }
    if (arg.tag() != tag::integer) {
      return throw_exception(type::type_error,
                             "Right-hand argument is not an Integer");
    }

    auto left = value::get<value::integer>(self);
    auto right = value::get<value::integer>(arg);
    return gc::alloc<value::boolean>( op(left, right) );
  };
}

bool boxed_integer_equal(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() == tag::floating_point) {
    auto left = value::get<value::integer>(self);
    auto right = value::get<value::floating_point>(arg);
    return left == right;
  }
  if (arg.tag() != tag::integer)
    return false;

  auto left = value::get<value::integer>(self);
  auto right = value::get<value::integer>(arg);
  return left == right;
}

}

// }}}

gc::managed_ptr integer::add(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_or_flt_op([](auto a, auto b) { return a + b; })(self, arg);
}

gc::managed_ptr integer::subtract(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_or_flt_op([](auto a, auto b) { return a - b; })(self, arg);
}

gc::managed_ptr integer::times(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_or_flt_op([](auto a, auto b) { return a * b; })(self, arg);
}

gc::managed_ptr integer::divides(gc::managed_ptr self, gc::managed_ptr arg)
{
  auto left = value::get<value::integer>(self);
  if (arg.tag() == tag::floating_point) {
    if (value::get<value::floating_point>(arg) == 0.0)
      return throw_exception(type::divide_by_zero_error, message::divide_by_zero);
    return gc::alloc<value::floating_point>( left / value::get<value::floating_point>(arg) );
  }

  if (arg.tag() != tag::integer) {
    return throw_exception(type::type_error,
                           "Right-hand argument is not an Integer");
  }

  if (value::get<value::integer>(arg) == 0)
    return throw_exception(type::divide_by_zero_error, message::divide_by_zero);
  return gc::alloc<value::integer>( left / value::get<value::integer>(arg));
}

gc::managed_ptr integer::modulo(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_integer_op([](auto a, auto b) { return a % b; })(self, arg);
}

gc::managed_ptr integer::pow(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() == tag::floating_point) {
    auto left = value::get<value::integer>(self);
    auto right = value::get<value::floating_point>(arg);
    return gc::alloc<value::floating_point>( std::pow(left, right) );
  }

  if (arg.tag() != tag::integer) {
    return throw_exception(type::type_error,
                           "Right-hand argument is not an Integer");
  }

  auto left = value::get<value::integer>(self);
  auto right = value::get<value::integer>(arg);

  if (right < 0)
    return gc::alloc<value::floating_point>( std::pow(left, right) );
  return gc::alloc<value::integer>( static_cast<int>(std::pow(left, right)) );
}

gc::managed_ptr integer::lshift(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_integer_op([](auto a, auto b) { return a << b; })(self, arg);
}

gc::managed_ptr integer::rshift(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_integer_op([](auto a, auto b) { return a >> b; })(self, arg);
}

gc::managed_ptr integer::bit_and(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_integer_op([](auto a, auto b) { return a & b; })(self, arg);
}

gc::managed_ptr integer::bit_or(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_integer_op([](auto a, auto b) { return a | b; })(self, arg);
}

gc::managed_ptr integer::x_or(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_integer_op([](auto a, auto b) { return a ^ b; })(self, arg);
}

gc::managed_ptr integer::equals(gc::managed_ptr left, gc::managed_ptr right)
{
  return gc::alloc<value::boolean>( boxed_integer_equal(left, right) );
}

gc::managed_ptr integer::unequal(gc::managed_ptr left, gc::managed_ptr right)
{
  return gc::alloc<value::boolean>( !boxed_integer_equal(left, right) );
}

gc::managed_ptr integer::greater(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_bool_op([](auto a, auto b) { return a > b; })(self, arg);
}

gc::managed_ptr integer::less(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_bool_op([](auto a, auto b) { return a < b; })(self, arg);
}

gc::managed_ptr integer::greater_equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_bool_op([](auto a, auto b) { return a >= b; })(self, arg);
}

gc::managed_ptr integer::less_equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_int_bool_op([](auto a, auto b) { return a <= b; })(self, arg);
}

gc::managed_ptr integer::negative(gc::managed_ptr self)
{
  return fn_integer_monop(std::negate<int>{})(self);
}

gc::managed_ptr integer::negate(gc::managed_ptr self)
{
  return fn_integer_monop([](auto i) { return ~i; })(self);
}

gc::managed_ptr integer::sqrt(gc::managed_ptr self)
{
  return fn_int_to_flt_monop([](auto i) { return std::sqrt(i); })(self);
}

gc::managed_ptr integer::sin(gc::managed_ptr self)
{
  return fn_int_to_flt_monop([](auto i) { return std::sin(i); })(self);
}

gc::managed_ptr integer::cos(gc::managed_ptr self)
{
  return fn_int_to_flt_monop([](auto i) { return std::cos(i); })(self);
}

gc::managed_ptr integer::tan(gc::managed_ptr self)
{
  return fn_int_to_flt_monop([](auto i) { return std::tan(i); })(self);
}

gc::managed_ptr integer::chr(gc::managed_ptr self)
{
  auto ord = value::get<value::integer>(self);
  if (ord < 0 || ord > 255) {
    return throw_exception(type::range_error,
                           message::out_of_range(0, 256, ord));
  }

  return gc::alloc<value::character>( static_cast<char>(ord) );
}
