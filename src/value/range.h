#ifndef VV_VALUE_RANGE_H
#define VV_VALUE_RANGE_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct range : public basic_object {
  range(basic_object* start, basic_object* end);
  range();

  basic_object* start;
  basic_object* end;
};

}

}

#endif
