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

const vv::symbol sym::self{"self"};
const vv::symbol sym::call{"call"};

const vv::symbol sym::start{"start"};
const vv::symbol sym::at_end{"at_end"};
const vv::symbol sym::get{"get"};
const vv::symbol sym::increment{"increment"};

const vv::symbol sym::add{"add"};
const vv::symbol sym::subtract{"subtract"};
const vv::symbol sym::times{"times"};
const vv::symbol sym::divides{"divides"};

const vv::symbol sym::equals{"equals"};
const vv::symbol sym::unequal{"unequal"};

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
  vm.push(self);
  vm.readm(method);
  vm.call(0);
  vm.run_cur_scope();
  auto val = vm.top();
  vm.pop(1);
  return { true, val };
}

template <typename F>
call_result fake_for_loop(vm::machine& vm, const F& inner)
{
  // Get iterator from range
  vm.arg(0);
  auto range = vm.top();
  auto iter = call_method(vm, range, sym::start).value;

  vm.arg(1);
  auto supplied_fn = vm.top();

  for (;;) {
    auto at_end = call_method(vm, iter, sym::at_end).value;
    if (truthy(at_end)) {
      vm.pop(1); // iter
      return { true, at_end };
    }

    auto next_item = call_method(vm, iter, sym::get).value;

    auto res = inner(vm, supplied_fn, next_item);
    if (res.completed) {
      vm.pop(1); // iter
      return res;
    }

    call_method(vm, iter, sym::increment);
  }
}

template <typename F>
call_result transformed_range(vm::machine& vm, const F& inner)
{
  return fake_for_loop(vm, [&](auto vm, auto* transform, auto* orig)
  {
    vm.push(orig);

    vm.push(transform);
    vm.call(1);
    vm.run_cur_scope();

    auto completed = inner(orig, vm.top());
    return call_result{ completed, vm.top() };
  });
}

// }}}
// I/O {{{

value::base* fn_print(vm::machine& vm)
{
  vm.arg(0);
  auto arg = vm.top();

  if (arg->type == &type::string)
    std::cout << static_cast<value::string*>(arg)->val;
  else
    std::cout << pretty_print(*arg, vm);
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
  vm.parr(0);
  auto array = static_cast<value::array*>(vm.top());

  fake_for_loop(vm, [array](auto& vm, auto* pred_fn, auto* item)
  {
    vm.push(item);
    vm.push(pred_fn);
    vm.call(1);
    vm.run_cur_scope();
    if (truthy(vm.top()))
      array->val.push_back(item);
    vm.pop(1);
    return call_result{ false, nullptr };
  });

  vm.pop(1); // array
  return array;
}

value::base* fn_map(vm::machine& vm)
{
  // Get pointer to empty Array
  vm.parr(0);
  auto mapped = static_cast<value::array*>(vm.top());

  transformed_range(vm, [mapped](auto*, auto* val)
                                { mapped->val.push_back(val); return false; });

  vm.pop(1); // filtered
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
  auto range = vm.top();
  vm.pop(1);
  auto iter = call_method(vm, range, sym::start).value;
  vm.push(iter);

  vm.arg(1); // put total on stack


  for (;;) {
    auto at_end = call_method(vm, iter, sym::at_end).value;
    if (truthy(at_end)) {
      return vm.top();
    }

    auto total = vm.top();
    auto next_item = call_method(vm, iter, sym::get).value;
    vm.push(next_item);
    vm.push(total);

    vm.arg(2); // supplied function
    vm.call(2);
    vm.run_cur_scope();

    // replace old total with new
    total = vm.top();
    vm.pop(2); // new total, orig total
    vm.push(total);

    call_method(vm, iter, sym::increment);
  }
}

value::base* fn_sort(vm::machine& vm)
{
  vm.parr(0);
  auto& array = static_cast<value::array&>(*vm.top());

  vm.arg(0);
  auto range = vm.top();
  vm.pop(1);
  auto iter = call_method(vm, range, sym::start).value;
  vm.push(iter);

  for (;;) {
    auto at_end = call_method(vm, iter, sym::at_end).value;
    if (truthy(at_end))
      break;

    auto next_item = call_method(vm, iter, sym::get).value;
    array.val.push_back(next_item);

    call_method(vm, iter, sym::increment);
  }

  std::sort(begin(array.val), end(array.val), [&](auto* left, auto* right)
  {
    vm.push(right);
    vm.push(left);
    vm.readm({"less"});
    vm.call(1);
    vm.run_cur_scope();
    auto res = vm.top();
    vm.pop(1);
    return truthy(res);
  });

  return &array;
}

// }}}
// Other {{{

value::base* fn_quit(vm::machine&)
{
  exit(0);
}

value::base* fn_reverse(vm::machine& vm)
{
  // Get iterator from range
  vm.arg(0);
  auto range = vm.top();
  auto iter = call_method(vm, range, sym::start).value;

  vm.parr(0);
  auto* arr = static_cast<value::array*>(vm.top());

  for (;;) {
    auto at_end = call_method(vm, iter, sym::at_end).value;
    if (truthy(at_end)) {
      vm.pop(1); // iter
      reverse(begin(arr->val), end(arr->val));
      return arr;
    }

    auto next_item = call_method(vm, iter, sym::get).value;
    arr->val.push_back(next_item);

    call_method(vm, iter, sym::increment);
  }
}

// }}}

}

value::builtin_function function::print{fn_print, 1};
value::builtin_function function::puts{ fn_puts,  1};
value::builtin_function function::gets{ fn_gets,  0};

value::builtin_function function::filter{fn_filter, 2};
value::builtin_function function::map   {fn_map,    2};
value::builtin_function function::reduce{fn_reduce, 3};
value::builtin_function function::sort  {fn_sort,   1};
value::builtin_function function::all   {fn_all,    2};
value::builtin_function function::any   {fn_any,    2};
value::builtin_function function::count {fn_count,  2};

value::builtin_function function::quit   {fn_quit,    0};
value::builtin_function function::reverse{fn_reverse, 1};

// }}}
// Types {{{

value::type type::function {[]{ return nullptr; }, {
}, builtin::type::object, {"Function"}};

// }}}

void builtin::make_base_env(vm::environment& base)
{
  base.members = {
    { {"print"},          &builtin::function::print },
    { {"puts"},           &builtin::function::puts },
    { {"gets"},           &builtin::function::gets },
    { {"count"},          &builtin::function::count },
    { {"filter"},         &builtin::function::filter },
    { {"map"},            &builtin::function::map },
    { {"reduce"},         &builtin::function::reduce },
    { {"sort"},           &builtin::function::sort },
    { {"any"},            &builtin::function::any },
    { {"all"},            &builtin::function::all },
    { {"quit"},           &builtin::function::quit },
    { {"reverse"},        &builtin::function::reverse },
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
    { {"RegEx"},          &builtin::type::regex },
    { {"RegExResult"},    &builtin::type::regex_result },
    { {"String"},         &builtin::type::string },
    { {"StringIterator"}, &builtin::type::string_iterator },
    { {"Symbol"},         &builtin::type::symbol },
    { {"Type"},           &builtin::type::custom_type }
  };
}
