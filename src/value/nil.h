#ifndef VV_VALUE_NIL_H
#define VV_VALUE_NIL_H

#include "value.h"

namespace vv {

namespace value {

struct nil : public object {
  nil();
  std::string value() const override;
};

}

}

#endif
