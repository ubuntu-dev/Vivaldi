#ifndef VV_VALUE_BLOB_H
#define VV_VALUE_BLOB_H

#include "value/basic_object.h"

#include <functional>

namespace vv {

namespace value {

struct blob : public basic_object {
public:
  blob(void* val, const std::function<void(blob*)>& dtor);

  blob(blob&& other);
  blob& operator=(blob&& other);

  struct value_type {
    void* val;
    std::function<void(blob*)> c_dtor;
  };

  value_type value;

  ~blob();
};

}

}

#endif
