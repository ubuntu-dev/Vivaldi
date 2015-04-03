#ifndef VV_VALUE_BLOB_H
#define VV_VALUE_BLOB_H

#include "value/basic_object.h"

#include <functional>

namespace vv {

namespace value {

struct blob : public basic_object {
public:
  blob(void* val, const std::function<void(basic_object*)>& dtor);

  blob(blob&& other);
  blob& operator=(blob&& other);

  void* val;
  std::function<void(basic_object*)> c_dtor;


  ~blob();
};

}

}

#endif
