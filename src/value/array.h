#ifndef VV_VALUE_ARRAY_H
#define VV_VALUE_ARRAY_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

struct array : public object {
public:
  // Creates an Array containing a copy of the provided vector.
  array(const std::vector<gc::managed_ptr<object>>& mems = {});

  std::string value() const override;
  void mark() override;

  std::vector<gc::managed_ptr<object>> val;
};

}

}

#endif
