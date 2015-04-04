#include "builtins/range.h"

#include "builtins.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr range::init(vm::machine& vm)
{
  vm.self();
  auto rng = vm.top();
  vm.arg(1);
  value::get<value::range>(rng).end = vm.top();
  vm.arg(0);
  value::get<value::range>(rng).start = vm.top();
  return rng;
}

gc::managed_ptr range::start(gc::managed_ptr self)
{
  return gc::alloc<value::range>( static_cast<value::range&>(*self.get()) );
}

gc::managed_ptr range::size(vm::machine& vm)
{
  vm.self();
  auto& rng = value::get<value::range>(vm.top());
  vm.push(rng.start);
  vm.push(rng.end);
  vm.opt_sub();
  return vm.top();
}

gc::managed_ptr range::at_end(vm::machine& vm)
{
  vm.self();
  auto& rng = value::get<value::range>(vm.top());
  vm.push(rng.start);
  vm.push(rng.end);
  vm.readm(sym::greater);
  vm.call(1);
  vm.run_cur_scope();
  vm.opt_not();
  return vm.top();
}

gc::managed_ptr range::get(gc::managed_ptr self)
{
  return value::get<value::range>(self).start;
}

gc::managed_ptr range::increment(vm::machine& vm)
{
  vm.self();
  auto rng = vm.top();
  vm.pint(1);
  vm.push(value::get<value::range>(rng).start);
  vm.opt_add();
  value::get<value::range>(rng).start = vm.top();
  return rng;
}

gc::managed_ptr range::to_arr(vm::machine& vm)
{
  vm.self();
  auto& rng = value::get<value::range>(vm.top());
  vm.push(rng.start);
  auto iter = vm.top();
  int count{};
  for (;;) {
    vm.dup();
    vm.push(rng.end);
    vm.readm(sym::greater);
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
