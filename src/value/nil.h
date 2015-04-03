#ifndef VV_VALUE_NIL_H
#define VV_VALUE_NIL_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct nil : public basic_object {
  nil();
};

}

}

#endif
