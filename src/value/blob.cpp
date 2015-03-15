#include "blob.h"

using namespace vv;
using namespace value;

blob::blob(void* val, const std::function<void(vv_object_t*)>& dtor)
  : val    {val},
    c_dtor {dtor}
{ }

blob::~blob()
{
  c_dtor(reinterpret_cast<vv_object_t*>(this));
}
