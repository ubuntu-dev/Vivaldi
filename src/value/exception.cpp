#include "value/exception.h"

#include "builtins.h"

using namespace vv;

value::exception::exception(const std::string& message)
  : basic_object {builtin::type::exception},
    value        {message}
{ }
