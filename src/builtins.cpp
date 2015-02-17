#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/function.h"
#include "value/array.h"
#include "value/string.h"

#include <iostream>

using namespace vv;
using namespace builtin;

// Symbols {{{

vv::symbol sym::self{"self"};
vv::symbol sym::call{"call"};

// }}}
// Freestanding functions {{{

namespace {

// Helper functions {{{

struct call_result {
  bool successful;
  value::base* result;
};

call_result call_method(std::shared_ptr<vm::call_frame> frame,
                        value::base* self,
                        symbol method)
{
  auto excepted = false;
  vm::machine machine{frame, [&](vm::machine&) { excepted = true; }};
  machine.retval = self;
  machine.readm(method);
  machine.call(0);
  machine.run();
  return { !excepted, machine.retval };
}

template <typename F>
boost::optional<value::base*> fake_for_loop(vm::machine& vm, const F& inner)
{
  auto range = get_arg(vm, 0);
  auto supplied_fn = get_arg(vm, 1);

  auto frame = std::make_shared<vm::call_frame>(nullptr,
                                                vm.frame,
                                                0,
                                                vector_ref<vm::command>{});
  // Get iterator from range
  auto iter_res = call_method(frame, range, {"start"});
  if (!iter_res.successful)
    return iter_res.result;

  auto iter = iter_res.result;

  for (;;) {
    auto at_end_res = call_method(frame, iter, {"at_end"});
    if (!at_end_res.successful)
      return at_end_res.result;

    if (truthy(at_end_res.result))
      return {};

    auto get_res = call_method(frame, iter, {"get"});
    if (!get_res.successful)
      return get_res.result;

    auto next_item = get_res.result;

    auto excepted = inner(frame, supplied_fn, next_item);
    if (excepted)
      return *excepted;

    auto increment_res = call_method(frame, iter, {"increment"});
    if (!increment_res.successful)
      return increment_res.result;
  }
}

template <typename F>
boost::optional<value::base*> transformed_range(vm::machine& vm, const F& inner)
{
  const static std::array<vm::command, 1> transform_call {{
    { vm::instruction::call, 1 }
  }};

  return fake_for_loop(vm, [&](auto frame, auto* transform, auto* orig)
  {
    frame->pushed.push_back(orig);
    auto excepted = false;
    vm::machine machine{frame, [&](vm::machine&) { excepted = true; }};
    machine.retval = transform;
    machine.call(1);
    machine.run();
    if (excepted)
      return boost::optional<value::base*>{machine.retval};
    inner(orig, machine.retval);
    return boost::optional<value::base*>{};
  });
}

// }}}
// I/O {{{

value::base* fn_print(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);

  if (arg->type == &type::string)
    std::cout << static_cast<value::string*>(arg)->val;
  else
    std::cout << arg->value();
  return gc::alloc<value::nil>( );
}

value::base* fn_puts(vm::machine& vm)
{
  auto ret = fn_print(vm);
  std::cout << '\n';
  return ret;
}

value::base* fn_gets(vm::machine&)
{
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

// }}}
// Functional stuff {{{

value::base* fn_filter(vm::machine& vm)
{
  // Get pointer to empty Array
  vm.make_arr(0);
  vm.push(); // Avoid GC'ing retval of inactive VM
  auto filtered = static_cast<value::array*>(vm.retval);

  auto excepted = transformed_range(vm, [&](auto* candidate, auto* pred)
  {
    if (truthy(pred))
      filtered->val.push_back(candidate);
  });
  if (excepted)
    return throw_exception(*excepted, vm);

  vm.pop(); // filtered
  return filtered;
}

value::base* fn_map(vm::machine& vm)
{
  // Get pointer to empty Array
  vm.make_arr(0);
  vm.push(); // Avoid GC'ing retval of inactive VM
  auto mapped = static_cast<value::array*>(vm.retval);

  auto excepted = transformed_range(vm, [&](auto*, auto* val)
                                           { mapped->val.push_back(val); });
  if (excepted)
    return throw_exception(*excepted, vm);

  vm.pop(); // filtered
  return mapped;
}

value::base* fn_count(vm::machine& vm)
{
  int count{};

  auto excepted = transformed_range(vm, [&](auto*, auto* pred)
  {
    if (truthy(pred))
      ++count;
  });
  if (excepted)
    return throw_exception(*excepted, vm);

  return gc::alloc<value::integer>( count );
}

// }}}
// Other {{{

value::base* fn_quit(vm::machine&)
{
  gc::empty();
  exit(0);
}

// }}}

}

value::builtin_function function::print{fn_print, 1};
value::builtin_function function::puts{ fn_puts,  1};
value::builtin_function function::gets{ fn_gets,  0};

value::builtin_function function::filter{fn_filter, 2};
value::builtin_function function::map   {fn_map,    2};
//value::builtin_function function::reduce{fn_reduce, 2};
//value::builtin_function function::all   {fn_all,    2};
//value::builtin_function function::any   {fn_any,    2};
value::builtin_function function::count {fn_count,  2};


value::builtin_function function::quit{fn_quit,  0};

// }}}
// Types {{{

value::type type::function {[]{ return nullptr; }, {
}, builtin::type::object, {"Function"}};

// }}}

void builtin::make_base_env(vm::call_frame& base)
{
  base.local.back() = {
    { {"print"},          &builtin::function::print },
    { {"puts"},           &builtin::function::puts },
    { {"gets"},           &builtin::function::gets },
    { {"count"},          &builtin::function::count },
    { {"filter"},         &builtin::function::filter },
    { {"map"},            &builtin::function::map },
    { {"quit"},           &builtin::function::quit },
    { {"Array"},          &builtin::type::array },
    { {"ArrayIterator"},  &builtin::type::array_iterator },
    { {"Bool"},           &builtin::type::boolean },
    { {"Dictionary"},     &builtin::type::dictionary },
    { {"File"},           &builtin::type::file },
    { {"Float"},          &builtin::type::floating_point },
    { {"Integer"},        &builtin::type::integer },
    { {"Nil"},            &builtin::type::nil },
    { {"Object"},         &builtin::type::object },
    { {"Range"},          &builtin::type::range },
    { {"String"},         &builtin::type::string },
    { {"StringIterator"}, &builtin::type::string_iterator },
    { {"Symbol"},         &builtin::type::symbol },
    { {"Type"},           &builtin::type::custom_type }
  };
}
