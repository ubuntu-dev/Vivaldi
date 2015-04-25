#ifndef VV_VALUE_METHOD_H
#define VV_VALUE_METHOD_H

#include "basic_object.h"

namespace vv {

namespace value {

struct method : public basic_object {
  method(gc::managed_ptr function, gc::managed_ptr self);
  struct value_type {
    gc::managed_ptr function;
    gc::managed_ptr self;
  };

  value_type value;
};

}

}

#endif
