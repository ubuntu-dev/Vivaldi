#ifndef VV_VALUE_FLOATING_POINT_H
#define VV_VALUE_FLOATING_POINT_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct floating_point : public basic_object {
public:
  floating_point(double val);

  double val;
};

}

}

#endif
