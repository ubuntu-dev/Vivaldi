#ifndef VV_UTILS_ERROR_H
#define VV_UTILS_ERROR_H

#include "value.h"

#include <stdexcept>

namespace vv {

class vm_error : public std::exception {
public:
  vm_error(value::basic_object* val) : m_val{val} { }
  value::basic_object* error() const { return m_val; }
private:
  value::basic_object* m_val;
};

}

#endif
