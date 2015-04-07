#include "blob.h"

#include "builtins.h"

using namespace vv;
using namespace value;

blob::blob(void* val, const std::function<void(void*)>& dtor)
  : basic_object {builtin::type::object},
    value        {val, dtor}
{ }

blob::blob(blob&& other)
  : basic_object {builtin::type::object},
    value        ( std::move(other.value) )
{
  other.value.val = nullptr;
  other.value.c_dtor = nullptr;
}

blob& blob::operator=(blob&& other)
{
  std::swap(value, other.value);
  return *this;
}

blob::~blob()
{
  if (value.c_dtor)
    value.c_dtor(value.val);
}
