#ifndef VV_VALUE_STRING_H
#define VV_VALUE_STRING_H

#include "value/basic_object.h"

#include <string>

namespace vv {

namespace value {

struct string : public basic_object {
  string(const std::string& val = "");

  using value_type = std::string;
  value_type value;
};

}

}

#endif
