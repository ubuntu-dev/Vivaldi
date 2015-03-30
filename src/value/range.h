#ifndef VV_VALUE_RANGE_H
#define VV_VALUE_RANGE_H

#include "value/object.h"

namespace vv {

namespace value {

struct range : public object {
  range(object* start, object* end);
  range();

  object* start;
  object* end;
};

}

}

#endif
