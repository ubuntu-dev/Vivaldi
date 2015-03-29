#ifndef VV_UTILS_ERROR_H
#define VV_UTILS_ERROR_H

#include "value.h"

#include <stdexcept>

namespace vv {

class vm_error : public std::exception {
public:
  vm_error(value::object* val) : m_val{val} { }
  value::object* error() const { return m_val; }
private:
  value::object* m_val;
};

}

#endif
