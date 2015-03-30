#ifndef VV_VALUE_BOOLEAN_H
#define VV_VALUE_BOOLEAN_H

#include "value/object.h"

namespace vv {

namespace value {

struct boolean : public object {
public:
  boolean(bool val = true);

  bool val;
};

}

}

#endif
