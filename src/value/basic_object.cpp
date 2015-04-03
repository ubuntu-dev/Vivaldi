#include "basic_object.h"

using namespace vv;
using namespace value;

basic_object::basic_object(value::type* type, const vv::tag tag)
  : tag  {tag},
    type {type}
{ }
