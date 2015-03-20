#ifndef VV_VALUE_STRING_ITERATOR_H
#define VV_VALUE_STRING_ITERATOR_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

struct string_iterator : public object {
public:
  string_iterator(gc::managed_ptr<string> str);
  string_iterator();

  std::string value() const override;
  void mark() override;

  gc::managed_ptr<string> str;
  size_t idx;
};

}

}

#endif
