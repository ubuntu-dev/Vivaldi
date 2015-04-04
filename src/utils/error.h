#ifndef VV_UTILS_ERROR_H
#define VV_UTILS_ERROR_H

#include "gc/managed_ptr.h"

#include <stdexcept>

namespace vv {

class vm_error : public std::exception {
public:
  vm_error(gc::managed_ptr val) : m_val{val} { }
  gc::managed_ptr error() const { return m_val; }
private:
  gc::managed_ptr m_val;
};

}

#endif
