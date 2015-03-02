#ifndef VV_VALUE_BUILTIN_FUNCTION_H
#define VV_VALUE_BUILTIN_FUNCTION_H

#include "value.h"
#include "vm.h"

namespace vv {

namespace value {

struct builtin_function : public basic_function {
public:
  builtin_function(const std::function<base*(vm::machine&)>& body, int argc);

  std::string value() const override;

  int get_argc() const override;
  vector_ref<vm::command> get_body() const override;
  vm::environment* get_enclosing() const override;

  std::function<base*(vm::machine&)> body;
  int argc;
};

}

}

#endif
