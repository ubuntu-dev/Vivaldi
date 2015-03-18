#ifndef VV_VALUE_BOOLEAN_H
#define VV_VALUE_BOOLEAN_H

#include "value.h"

namespace vv {

namespace value {

struct boolean : public object {
public:
  boolean(bool val = true);

  std::string value() const override;
  size_t hash() const override;
  bool equals(const object& other) const override;

  bool val;
};

}

}

#endif
