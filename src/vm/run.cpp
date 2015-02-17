#include "run.h"

#include "vm.h"

using namespace vv;

vm::run_result vm::run(std::shared_ptr<call_frame> in, value::base* startval)

{
  // Set up base env for VM, including fake parent that just holds arguments.
  // This is getting kind of expensive...

  // Flag to check for exceptions--- slightly hacky, but oh well
  auto excepted = false;
  vm::machine machine{in, [&](vm::machine&) { excepted = true; }};
  machine.retval = startval;
  machine.run();

  return { !excepted, machine };
}
