#ifndef VV_VM_RUN_H
#define VV_VM_RUN_H

#include "vm.h"

#include <string>

namespace vv {

namespace vm {

struct run_result {
  bool successful;
  vm::machine machine;
};

run_result run(std::shared_ptr<call_frame> in, value::base* startval = nullptr);

}

}

#endif
