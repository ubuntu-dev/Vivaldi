#ifndef VV_UTILS_ERROR_H
#define VV_UTILS_ERROR_H

#include "value.h"

#include <stdexcept>

namespace vv {

class vm_error : public std::exception {
public:
  vm_error(value::object_ptr val) : m_val{val} { }
  value::object_ptr error() const { return m_val; }
private:
  value::object_ptr m_val;
};

}

#endif
