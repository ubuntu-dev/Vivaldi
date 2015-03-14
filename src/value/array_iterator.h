#ifndef VV_VALUE_ARRAY_ITERATOR_H
#define VV_VALUE_ARRAY_ITERATOR_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

/// Vivaldi class for iterating through an Array.
struct array_iterator : public base {
public:
  /// Constructs an array_iterator pointing to the start of `arr`.
  array_iterator(array& arr);

  std::string value() const override;
  void mark() override;

  /// Owning array.
  array& arr;
  /// Position within arr.

  /// This is Implemented as size_t, and not as an std::vector iterator, so as
  /// to persist through vector resizes.
  size_t idx;
};

}

}

#endif
