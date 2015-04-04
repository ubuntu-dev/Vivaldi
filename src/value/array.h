#ifndef VV_VALUE_ARRAY_H
#define VV_VALUE_ARRAY_H

#include "value/basic_object.h"
#include "expression.h"

namespace vv {

namespace value {

struct array : public basic_object {
public:
  // Creates an Array containing a copy of the provided vector.
  array(const std::vector<gc::managed_ptr>& mems = {});

  using value_type = std::vector<gc::managed_ptr>;
  value_type value;
};

}

}

#endif
