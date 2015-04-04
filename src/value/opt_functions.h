#ifndef VV_VALUE_OPT_FUNCTIONS_H
#define VV_VALUE_OPT_FUNCTIONS_H

#include "value/basic_object.h"
#include "vm.h"

namespace vv {

namespace value {

struct opt_monop : public basic_object {
public:
  opt_monop(const std::function<gc::managed_ptr(gc::managed_ptr)>& body);

  struct value_type {
    std::function<gc::managed_ptr(gc::managed_ptr)> body;
  };

  value_type value;
};

struct opt_binop : public basic_object {
public:
  opt_binop(const std::function<gc::managed_ptr(gc::managed_ptr, gc::managed_ptr)>& body);

  struct value_type {
    std::function<gc::managed_ptr(gc::managed_ptr, gc::managed_ptr)> body;
  };

  value_type value;
};

}

}

#endif
