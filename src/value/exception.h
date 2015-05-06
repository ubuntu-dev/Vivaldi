#ifndef VV_VALUE_EXCEPTION_H
#define VV_VALUE_EXCEPTION_H

#include "value/basic_object.h"

namespace vv {

namespace value {

struct exception : public basic_object {
  exception(const std::string& message = "");

  struct value_type {
    std::string message;
  };

  value_type value;
};

}

}

#endif
