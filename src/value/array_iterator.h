#ifndef VV_VALUE_ARRAY_ITERATOR_H
#define VV_VALUE_ARRAY_ITERATOR_H

#include "value/basic_object.h"
#include "expression.h"

namespace vv {

namespace value {

// Vivaldi class for iterating through an Array.
struct array_iterator : public basic_object {
public:
  // Constructs an array_iterator pointing to the start of arr.
  array_iterator(gc::managed_ptr arr);

  struct value_type {
    // Owning array.
    gc::managed_ptr arr;
    // Position within arr. Implemented as size_t, and not as an std::vector
    // iterator, so as to persist through vector resizes.
    size_t idx;
  };

  value_type value;
};

}

}

#endif
