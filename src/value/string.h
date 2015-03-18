#ifndef VV_VALUE_STRING_H
#define VV_VALUE_STRING_H

#include "value.h"

#include <string>

namespace vv {

namespace value {

struct string : public object {
public:
  string(const std::string& val = "");

  std::string value() const override;
  size_t hash() const override;
  bool equals(const object& other) const override;

  std::string val;
};

}

}

#endif
