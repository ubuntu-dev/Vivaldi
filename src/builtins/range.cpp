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
  auto& rng = static_cast<value::range&>(*vm.retval);
  vm.arg(1);
  rng.end = vm.retval;
  vm.arg(0);
  rng.start = vm.retval;
  return &rng;
}

value::base* fn_range_start(vm::machine& vm)
{
  vm.self();
  return vm.retval;
}

value::base* fn_range_size(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.retval);
  vm.retval = rng.start;
  vm.push_arg();
  vm.retval = rng.end;
  vm.readm({"subtract"});
  vm.call(1);
  vm.run_cur_scope();
  return vm.retval;
}

value::base* fn_range_at_end(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.retval);
  vm.retval = rng.start;
  vm.push_arg();
  vm.retval = rng.end;
  vm.readm({"greater"});
  vm.call(1);
  vm.run_cur_scope();
  vm.readm({"not"});
  vm.call(0);
  vm.run_cur_scope();
  return vm.retval;
}

value::base* fn_range_get(vm::machine& vm)
{
  vm.self();
  return static_cast<value::range&>(*vm.retval).start;
}

value::base* fn_range_increment(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.retval);
  vm.push_int(1);
  vm.push_arg();
  vm.retval = rng.start;
  vm.readm({"add"});
  vm.call(1);
  vm.run_cur_scope();
  rng.start = vm.retval;
  return &rng;
}

value::base* fn_range_to_arr(vm::machine& vm)
{
  vm.self();
  auto& rng = static_cast<value::range&>(*vm.retval);
  std::vector<value::base*> vals;
  auto iter = vm.retval = rng.start;
  for (;;) {
    vm.push_arg();
    vm.retval = rng.end;
    vm.readm({"greater"});
    vm.call(1);
    vm.run_cur_scope();
    if (!truthy(vm.retval))
      break;
    vals.push_back(iter);
    vm.retval = iter;
    vm.push();
    vm.push_int(1);
    vm.push_arg();
    vm.retval = iter;
    vm.readm({"add"});
    vm.call(1);
    vm.run_cur_scope();
    iter = vm.retval;
  }

  auto ret = gc::alloc<value::array>( vals );
  for (auto i = vals.size(); i--;)
    vm.pop();
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

