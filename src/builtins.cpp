#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
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
  bool completed;
  value::base* value;
};

call_result call_method(vm::machine& vm, value::base* self, symbol method)
{
  vm.retval = self;
  vm.readm(method);
  vm.call(0);
  vm.run_cur_scope();
  return { true, vm.retval };
}

template <typename F>
call_result fake_for_loop(vm::machine& vm, const F& inner)
{
  // Get iterator from range
  vm.arg(0);
  auto range = vm.retval;
  auto iter = call_method(vm, range, {"start"}).value;
  vm.push();

  vm.arg(1);
  auto supplied_fn = vm.retval;

  for (;;) {
    auto at_end = call_method(vm, iter, {"at_end"}).value;
    if (truthy(at_end)) {
      vm.pop(); // iter
      return { true, at_end };
    }

    auto next_item = call_method(vm, iter, {"get"}).value;

    auto res = inner(vm, supplied_fn, next_item);
    if (res.completed) {
      vm.pop(); // iter
      return res;
    }

    call_method(vm, iter, {"increment"});
  }
}

template <typename F>
call_result transformed_range(vm::machine& vm, const F& inner)
{
  return fake_for_loop(vm, [&](auto vm, auto* transform, auto* orig)
  {
    vm.retval = orig;
    vm.push();

    vm.retval = transform;
    vm.call(1);
    vm.run_cur_scope();

    auto completed = inner(orig, vm.retval);
    return call_result{ completed, vm.retval };
  });
}

// }}}
// I/O {{{

value::base* fn_print(vm::machine& vm)
{
  vm.arg(0);
  auto arg = vm.retval;

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

  transformed_range(vm, [&](auto* cand, auto* pred)
                           {
                             if (truthy(pred))
                               filtered->val.push_back(cand);
                             return false;
                           });

  vm.pop(); // filtered
  return filtered;
}

value::base* fn_map(vm::machine& vm)
{
  // Get pointer to empty Array
  vm.make_arr(0);
  vm.push(); // Avoid GC'ing retval of inactive VM
  auto mapped = static_cast<value::array*>(vm.retval);

  transformed_range(vm, [&](auto*, auto* val)
                           { mapped->val.push_back(val); return false; });

  vm.pop(); // filtered
  return mapped;
}

value::base* fn_count(vm::machine& vm)
{
  int count{};
  transformed_range(vm, [&](auto*, auto* pred)
                           { if (truthy(pred)) ++count; return false; });
  return gc::alloc<value::integer>( count );
}

value::base* fn_all(vm::machine& vm)
{
  auto all = true;
  transformed_range(vm, [&](auto*, auto* pred)
                           { if (!truthy(pred)) all = false; return !all; });
  return gc::alloc<value::boolean>( all );
}

value::base* fn_any(vm::machine& vm)
{
  auto found = false;
  transformed_range(vm, [&](auto*, auto* pred)
                           { if (truthy(pred)) found = true; return found; });
  return gc::alloc<value::boolean>( found );
}

value::base* fn_reduce(vm::machine& vm)
{
  // Get iterator from range
  vm.arg(0);
  auto range = vm.retval;
  auto iter = call_method(vm, range, {"start"}).value;

  vm.arg(1);
  vm.push(); // store total locally
  vm.arg(2);
  auto supplied_fn = vm.retval;

  for (;;) {
    auto at_end = call_method(vm, iter, {"at_end"}).value;
    if (truthy(at_end)) {
      vm.pop(); // get total
      return vm.retval;
    }

    auto next_item = call_method(vm, iter, {"get"}).value;
    vm.pop(); // get total
    vm.push_arg();
    vm.retval = next_item;
    vm.push_arg();
    vm.retval = supplied_fn;
    vm.call(2);
    vm.run_cur_scope();
    vm.push(); // push total

    call_method(vm, iter, {"increment"});
  }
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
value::builtin_function function::reduce{fn_reduce, 3};
value::builtin_function function::all   {fn_all,    2};
value::builtin_function function::any   {fn_any,    2};
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
    { {"reduce"},         &builtin::function::reduce },
    { {"any"},            &builtin::function::any },
    { {"all"},            &builtin::function::all },
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
