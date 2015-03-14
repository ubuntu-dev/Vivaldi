#ifndef VV_VALUE_BOOLEAN_H
#define VV_VALUE_BOOLEAN_H

#include "value.h"

namespace vv {

namespace value {

/// Vivaldi Bool class.
struct boolean : public base {
public:
  /// Constructs a Bool with value `val`.
  boolean(bool val = true);

  std::string value() const override;
  size_t hash() const override;
  bool equals(const base& other) const override;

  /// Native value.
  bool val;
};

}

}

#endif
