#include "builtins/floating_point.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/floating_point.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

// Helper functions {{{

namespace {

bool is_float(gc::managed_ptr boxed) noexcept
{
  return boxed.tag() == tag::floating_point || boxed.tag() == tag::integer;
}

double to_float(gc::managed_ptr boxed) noexcept
{
  if (boxed.tag() == tag::floating_point)
    return value::get<value::floating_point>(boxed);
  return static_cast<double>(value::get<value::integer>(boxed));
}

template <typename F>
auto fn_float_op(const F& op)
{
  return [=](gc::managed_ptr self, gc::managed_ptr arg)
  {
    if (!is_float(arg)) {
      return throw_exception(type::type_error,
                             "Right-hand argument is not a Float");
    }

    auto res = gc::alloc<value::floating_point>(op(to_float(self), to_float(arg)));
    return static_cast<gc::managed_ptr>(res);
  };
}

template <typename F>
auto fn_float_bool_op(const F& op)
{
  return [=](gc::managed_ptr self, gc::managed_ptr arg)
  {
    if (!is_float(arg)) {
      return throw_exception(type::type_error,
                             "Right-hand argument is not a Float");
    }

    auto res = gc::alloc<value::boolean>( op(to_float(self), to_float(arg)) );
    return static_cast<gc::managed_ptr>(res);
  };
}

template <typename F>
auto fn_float_monop(const F& op)
{
  return [=](gc::managed_ptr self)
  {
    return gc::alloc<value::floating_point>( op(to_float(self)) );
  };
}

}

// }}}

gc::managed_ptr floating_point::add(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_op([](auto a, auto b) { return a + b; })(self, arg);
}

gc::managed_ptr floating_point::subtract(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_op([](auto a, auto b) { return a - b; })(self, arg);
}

gc::managed_ptr floating_point::times(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_op([](auto a, auto b) { return a * b; })(self, arg);
}

gc::managed_ptr floating_point::divides(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (!is_float(arg)) {
    return throw_exception(type::type_error,
                           "Right-hand argument is not a Float");
  }

  if (to_float(arg) == 0)
    return throw_exception(type::divide_by_zero_error, message::divide_by_zero);
  return gc::alloc<value::floating_point>( to_float(self) / to_float(arg) );
}

gc::managed_ptr floating_point::pow(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_op([](auto a, auto b) { return std::pow(a, b); })(self, arg);
}

gc::managed_ptr floating_point::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (!is_float(arg))
    return gc::alloc<value::boolean>( false );

  return fn_float_bool_op([](auto a, auto b) { return a == b; })(self, arg);
}

gc::managed_ptr floating_point::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (!is_float(arg))
    return gc::alloc<value::boolean>( true );

  return fn_float_bool_op([](auto a, auto b) { return a != b; })(self, arg);
}

gc::managed_ptr floating_point::less(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_bool_op([](auto a, auto b) { return a < b; })(self, arg);
}

gc::managed_ptr floating_point::greater(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_bool_op([](auto a, auto b) { return a > b; })(self, arg);
}

gc::managed_ptr floating_point::less_equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_bool_op([](auto a, auto b) { return a <= b; })(self, arg);
}

gc::managed_ptr floating_point::greater_equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  return fn_float_bool_op([](auto a, auto b) { return a >= b; })(self, arg);
}

gc::managed_ptr floating_point::to_int(gc::managed_ptr self)
{
  const auto float_val = value::get<value::floating_point>(self);
  return gc::alloc<value::integer>( static_cast<value::integer>(float_val) );
}

gc::managed_ptr floating_point::negative(gc::managed_ptr self)
{
  return fn_float_monop([](auto a) { return -a; })(self);
}

gc::managed_ptr floating_point::sqrt(gc::managed_ptr self)
{
  return fn_float_monop([](auto a) { return std::sqrt(a); })(self);
}

gc::managed_ptr floating_point::sin(gc::managed_ptr self)
{
  return fn_float_monop([](auto a) { return std::sin(a); })(self);
}

gc::managed_ptr floating_point::cos(gc::managed_ptr self)
{
  return fn_float_monop([](auto a) { return std::cos(a); })(self);
}

gc::managed_ptr floating_point::tan(gc::managed_ptr self)
{
  return fn_float_monop([](auto a) { return std::tan(a); })(self);
}
