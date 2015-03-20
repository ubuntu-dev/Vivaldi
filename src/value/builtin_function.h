#ifndef VV_VALUE_BUILTIN_FUNCTION_H
#define VV_VALUE_BUILTIN_FUNCTION_H

#include "value.h"
#include "vm.h"

namespace vv {

namespace value {

struct builtin_function : public basic_function {
public:
  builtin_function(const std::function<object_ptr(vm::machine&)>& body, int argc);

  std::string value() const override;

  std::function<object_ptr(vm::machine&)> fn_body;
};

}

}

#endif
