#ifndef VV_VALUE_BUILTIN_FUNCTION_H
#define VV_VALUE_BUILTIN_FUNCTION_H

#include "value/basic_function.h"
#include "vm.h"

namespace vv {

namespace value {

struct builtin_function : public basic_function {
public:
  builtin_function(const std::function<object*(vm::machine&)>& body, int argc);

  std::function<object*(vm::machine&)> fn_body;
};

}

}

#endif
