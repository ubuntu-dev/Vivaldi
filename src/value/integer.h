#ifndef VV_VALUE_INTEGER_H
#define VV_VALUE_INTEGER_H

#include "value/object.h"

namespace vv {

namespace value {

struct integer : public object {
public:
  integer(int val = 0);

  int val;
};

}

}

#endif
