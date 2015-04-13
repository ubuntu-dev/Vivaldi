#ifndef VV_VALUE_PARTIAL_FUNCTION_H
#define VV_VALUE_PARTIAL_FUNCTION_H

#include "basic_object.h"

namespace vv {

namespace value {

struct partial_function : public basic_object {
  partial_function(gc::managed_ptr function,
                   const std::vector<gc::managed_ptr>& provided_args);
  struct value_type {
    gc::managed_ptr function;
    std::vector<gc::managed_ptr> provided_args;
  };

  value_type value;
};

}

}

#endif
