#ifndef VV_VALUE_BLOB_H
#define VV_VALUE_BLOB_H

#include "value.h"
#include "vivaldi.h"

namespace vv {

namespace value {

struct blob : public base {
public:
  blob(void* val, const std::function<void(vv_object_t*)>& dtor);

  void* val;
  std::function<void(vv_object_t*)> c_dtor;


  ~blob();
};

}

}

#endif
