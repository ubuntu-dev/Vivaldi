#ifndef VV_VALUE_BLOB_H
#define VV_VALUE_BLOB_H

#include "value.h"

namespace vv {

namespace value {

struct blob : public base {
public:
  blob(void* val, const std::function<void(base*)>& dtor);

  blob(blob&& other);
  blob& operator=(blob&& other);

  void* val;
  std::function<void(base*)> c_dtor;


  ~blob();
};

}

}

#endif
