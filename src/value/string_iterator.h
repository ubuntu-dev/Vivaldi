#ifndef VV_VALUE_STRING_ITERATOR_H
#define VV_VALUE_STRING_ITERATOR_H

#include "value/basic_object.h"
#include "expression.h"

namespace vv {

namespace value {

struct string_iterator : public basic_object {
public:
  string_iterator(gc::managed_ptr str);
  string_iterator();

  struct value_type {
    gc::managed_ptr str;
    size_t idx;
  };
  value_type value;
};

}

}

#endif
