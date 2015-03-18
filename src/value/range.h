#ifndef VV_VALUE_RANGE_H
#define VV_VALUE_RANGE_H

#include "value.h"

namespace vv {

namespace value {

struct range : public object {
  range(object& start, object& end);
  range();
  std::string value() const override;

  object* start;
  object* end;

  void mark() override;
};

}

}

#endif
