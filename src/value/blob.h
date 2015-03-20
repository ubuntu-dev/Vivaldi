#ifndef VV_VALUE_BLOB_H
#define VV_VALUE_BLOB_H

#include "value.h"

namespace vv {

namespace value {

struct blob : public object {
public:
  blob(void* val, const std::function<void(object_ptr)>& dtor);

  blob(blob&& other);
  blob& operator=(blob&& other);

  void* val;
  std::function<void(object_ptr)> c_dtor;


  ~blob();
};

}

}

#endif
