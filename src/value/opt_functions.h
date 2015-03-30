#ifndef VV_VALUE_OPT_FUNCTIONS_H
#define VV_VALUE_OPT_FUNCTIONS_H

#include "value/basic_function.h"
#include "vm.h"

namespace vv {

namespace value {

struct opt_monop : public basic_function {
public:
  opt_monop(const std::function<object*(object*)>& body);

  std::function<object*(object*)> fn_body;
};

struct opt_binop : public basic_function {
public:
  opt_binop(const std::function<object*(object*, object*)>& body);

  std::function<object*(object*, object*)> fn_body;
};

}

}

#endif
