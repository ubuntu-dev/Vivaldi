#include "basic_object.h"

using namespace vv;
using namespace value;

basic_object::basic_object(const gc::managed_ptr type)
  : type {type}
{ }
