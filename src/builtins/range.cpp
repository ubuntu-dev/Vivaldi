#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/range.h"

using namespace vv;
using namespace builtin;

namespace {

value::base* fn_range_init(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.arg(1);
  rng.end = vm.top();
  vm.arg(0);
  rng.start = vm.top();
  return &rng;
}

value::base* fn_range_start(value::base* self)
{
  return gc::alloc<value::range>(*static_cast<value::range*>(self));
}

value::base* fn_range_size(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.push(rng.start);
  vm.push(rng.end);
  vm.opt_sub();
  return vm.top();
}

value::base* fn_range_at_end(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.push(rng.start);
  vm.push(rng.end);
  vm.readm({"greater"});
  vm.call(1);
  vm.run_cur_scope();
  vm.opt_not();
  return vm.top();
}

value::base* fn_range_get(value::base* self)
{
  return static_cast<value::range*>(self)->start;
}

value::base* fn_range_increment(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.pint(1);
  vm.push(rng.start);
  vm.opt_add();
  rng.start = vm.top();
  return &rng;
}

value::base* fn_range_to_arr(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.push(rng.start);
  auto iter = vm.top();
  int count{};
  for (;;) {
    vm.dup();
    vm.push(rng.end);
    vm.readm({"greater"});
    vm.call(1);
    vm.run_cur_scope();
    if (!truthy(vm.top()))
      break;
    vm.pop(1);
    vm.pint(1);
    vm.push(iter);
    vm.opt_add();
    iter = vm.top();
    ++count;
  }

  vm.pop(2);
  vm.parr(static_cast<int>(count));
  return vm.top();
}

value::builtin_function range_init      {fn_range_init,      2};
value::opt_monop        range_start     {fn_range_start       };
value::builtin_function range_size      {fn_range_size,      0};
value::builtin_function range_at_end    {fn_range_at_end,    0};
value::opt_monop        range_get       {fn_range_get         };
value::builtin_function range_increment {fn_range_increment, 0};
value::builtin_function range_to_arr    {fn_range_to_arr,    0};

}

value::type type::range {gc::alloc<value::range>, {
  { {"init"},      &range_init },
  { {"start"},     &range_start },
  { {"size"},      &range_size },
  { {"at_end"},    &range_at_end },
  { {"get"},       &range_get },
  { {"increment"}, &range_increment },
  { {"to_arr"},    &range_to_arr },
}, builtin::type::object, {"Range"}};

