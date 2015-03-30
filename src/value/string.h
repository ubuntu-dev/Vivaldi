#ifndef VV_VALUE_STRING_H
#define VV_VALUE_STRING_H

#include "value/object.h"

#include <string>

namespace vv {

namespace value {

struct string : public object {
  string(const std::string& val = "");

  std::string val;
};

}

}

#endif
