#ifndef VV_VALUE_OPT_FUNCTIONS_H
#define VV_VALUE_OPT_FUNCTIONS_H

#include "value.h"
#include "vm.h"

namespace vv {

namespace value {

struct opt_monop : public basic_function {
public:
  opt_monop(const std::function<object_ptr(object_ptr)>& body);

  std::string value() const override;

  std::function<object_ptr(object_ptr)> fn_body;
};

struct opt_binop : public basic_function {
public:
  opt_binop(const std::function<object_ptr(object_ptr, object_ptr)>& body);

  std::string value() const override;

  std::function<object_ptr(object_ptr, object_ptr)> fn_body;
};

}

}

#endif
