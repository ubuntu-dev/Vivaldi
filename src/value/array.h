#ifndef VV_VALUE_ARRAY_H
#define VV_VALUE_ARRAY_H

#include "value.h"
#include "expression.h"

namespace vv {

namespace value {

/// C++ representation of the Vivaldi Array class.
struct array : public base {
public:
  /// Creates an Array containing a copy of the provided vector.
  /// \param mems Array to copy.
  array(const std::vector<base*>& mems = {});

  /// Provides a string representation of the Array.
  std::string value() const override;
  void mark() override;

  /// Members of the Array.
  std::vector<base*> val;
};

}

}

#endif
