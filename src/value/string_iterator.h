#ifndef VV_VALUE_STRING_ITERATOR_H
#define VV_VALUE_STRING_ITERATOR_H

#include "value/basic_object.h"
#include "expression.h"

namespace vv {

namespace value {

struct string_iterator : public basic_object {
public:
  string_iterator(string& str);
  string_iterator();

  string& str;
  size_t idx;
};

}

}

#endif
