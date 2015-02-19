#ifndef VV_UTILS_ERROR_H
#define VV_UTILS_ERROR_H

#include "value.h"

#include <stdexcept>

namespace vv {

class vm_error : public std::exception {
public:
  vm_error(value::base* val) : m_val{val} { }
  value::base* error() const { return m_val; }
private:
  value::base* m_val;
};

}

#endif
