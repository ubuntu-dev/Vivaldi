#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/function.h"
#include "value/array.h"
#include "value/string.h"
#include "vm/run.h"

#include <iostream>

using namespace vv;
using namespace builtin;

// Symbols {{{

vv::symbol sym::self{"self"};
vv::symbol sym::call{"call"};

// }}}
// Freestanding functions {{{

namespace {

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

template <typename F>
boost::optional<value::base*> fake_for_loop(vm::machine& vm, const F& inner)
{
  const static std::array<vm::command, 2> get_iter_call {{
    { vm::instruction::readm, symbol{"start"} },
    { vm::instruction::call, 0 }
  }};
  const static std::array<vm::command, 2> at_end_call {{
    { vm::instruction::readm, symbol{"at_end"} },
    { vm::instruction::call, 0 }
  }};
  const static std::array<vm::command, 2> get_call {{
    { vm::instruction::readm, symbol{"get"} },
    { vm::instruction::call, 0 }
  }};
  const static std::array<vm::command, 2> increment_call {{
    { vm::instruction::readm, symbol{"increment"} },
    { vm::instruction::call, 0 }
  }};

  auto range = get_arg(vm, 0);
  auto supplied_fn = get_arg(vm, 1);

  auto frame = std::make_shared<vm::call_frame>(nullptr,
                                                vm.frame,
                                                0,
                                                vector_ref<vm::command>{});
  // Get iterator from range
  frame->instr_ptr = get_iter_call;
  auto iter_res = vm::run(frame, range);
  if (!iter_res.successful)
    return iter_res.machine.retval;
  frame->pushed.push_back(iter_res.machine.retval); // Don't GC iterator!
  auto iter = iter_res.machine.retval;

  for (;;) {
    frame->instr_ptr = at_end_call;
    auto check_end_res = vm::run(frame, iter);
    if (!check_end_res.successful)
      return check_end_res.machine.retval;
    if (truthy(check_end_res.machine.retval))
      return {};

    frame->instr_ptr = get_call;
    auto get_res = vm::run(frame, iter);
    if (!get_res.successful)
      return get_res.machine.retval;
    auto next_item = get_res.machine.retval;

    auto excepted = inner(frame, supplied_fn, next_item);
    if (excepted)
      return *excepted;

    frame->instr_ptr = increment_call;
    auto increment_res = vm::run(frame, iter);
    if (!increment_res.successful)
      return increment_res.machine.retval;
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
    frame->instr_ptr = transform_call;
    auto transform_res = vm::run(frame, transform);
    if (!transform_res.successful)
      return boost::optional<value::base*>{transform_res.machine.retval};
    inner(orig, transform_res.machine.retval);
    return boost::optional<value::base*>{};
  });
}

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

value::base* fn_quit(vm::machine&)
{
  gc::empty();
  exit(0);
}

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
