#ifndef VV_VALUE_RANGE_H
#define VV_VALUE_RANGE_H

#include "value.h"

namespace vv {

namespace value {

struct range : public object {
  range(object_ptr start, object_ptr end);
  range();
  std::string value() const override;

  object_ptr start;
  object_ptr end;

  void mark() override;
};

}

}

#endif
