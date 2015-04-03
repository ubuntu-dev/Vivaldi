#ifndef VV_VALUE_BOOLEAN_H
#define VV_VALUE_BOOLEAN_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct boolean : public basic_object {
public:
  boolean(bool val = true);

  bool val;
};

}

}

#endif
