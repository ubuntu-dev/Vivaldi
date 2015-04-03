#ifndef VV_VALUE_INTEGER_H
#define VV_VALUE_INTEGER_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct integer : public basic_object {
public:
  integer(int val = 0);

  int val;
};

}

}

#endif
