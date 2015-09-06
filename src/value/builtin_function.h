#ifndef VV_VALUE_BUILTIN_FUNCTION_H
#define VV_VALUE_BUILTIN_FUNCTION_H

#include "value/basic_object.h"
#include "vm.h"

namespace vv {

namespace value {

struct builtin_function : public basic_object {
public:
  builtin_function(const std::function<gc::managed_ptr(vm::machine&)>& body,
                   size_t argc);

  struct value_type {
    std::function<gc::managed_ptr(vm::machine&)> body;
    size_t argc;
    bool takes_varargs;
  };

  value_type value;
};

}

}

#endif
