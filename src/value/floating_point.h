#ifndef VV_VALUE_FLOATING_POINT_H
#define VV_VALUE_FLOATING_POINT_H

#include "value/object.h"

namespace vv {

namespace value {

struct floating_point : public object {
public:
  floating_point(double val);

  double val;
};

}

}

#endif
