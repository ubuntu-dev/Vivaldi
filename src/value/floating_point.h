#ifndef VV_VALUE_FLOATING_POINT_H
#define VV_VALUE_FLOATING_POINT_H

#include "value.h"

namespace vv {

namespace value {

struct floating_point : public object {
public:
  floating_point(double val);

  std::string value() const override;
  size_t hash() const override;
  bool equals(const object& other) const override;

  double val;
};

}

}

#endif
