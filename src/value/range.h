#ifndef VV_VALUE_RANGE_H
#define VV_VALUE_RANGE_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct range : public basic_object {
  range(gc::managed_ptr start, gc::managed_ptr end);
  range();

  struct value_type {
    gc::managed_ptr start;
    gc::managed_ptr end;
  };

  value_type value;
};

}

}

#endif
