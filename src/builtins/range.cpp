#include "builtins.h"

#include "gc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
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

value::base* fn_range_start(vm::machine& vm)
{
  vm.self();
  return vm.top();
}

value::base* fn_range_size(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.push(rng.start);
  vm.push(rng.end);
  vm.readm({"subtract"});
  vm.call(1);
  vm.run_cur_scope();
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
  vm.readm({"not"});
  vm.call(0);
  vm.run_cur_scope();
  return vm.top();
}

value::base* fn_range_get(vm::machine& vm)
{
  vm.self();
  return static_cast<value::range&>(*vm.top()).start;
}

value::base* fn_range_increment(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  vm.pint(1);
  vm.push(rng.start);
  vm.readm({"add"});
  vm.call(1);
  vm.run_cur_scope();
  rng.start = vm.top();
  return &rng;
}

value::base* fn_range_to_arr(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.top());
  std::vector<value::base*> vals;
  vm.push(rng.start);
  auto iter = vm.top();
  for (;;) {
    vm.push(rng.end);
    vm.readm({"greater"});
    vm.call(1);
    vm.run_cur_scope();
    if (!truthy(vm.top()))
      break;
    vals.push_back(iter);
    vm.push(iter);
    vm.pint(1);
    vm.push(iter);
    vm.readm({"add"});
    vm.call(1);
    vm.run_cur_scope();
    iter = vm.top();
  }

  auto ret = gc::alloc<value::array>( vals );
  for (auto i = vals.size(); i--;)
    vm.pop(1);
  return ret;
}

value::builtin_function range_init      {fn_range_init,      2};
value::builtin_function range_start     {fn_range_start,     0};
value::builtin_function range_size      {fn_range_size,      0};
value::builtin_function range_at_end    {fn_range_at_end,    0};
value::builtin_function range_get       {fn_range_get,       0};
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

