#include "blob.h"

using namespace vv;
using namespace value;

blob::blob(void* val, const std::function<void(base*)>& dtor)
  : val    {val},
    c_dtor {dtor}
{ }

blob::blob(blob&& other)
  : val    {other.val},
    c_dtor {std::move(other.c_dtor)}
{
  other.val = nullptr;
  other.c_dtor = nullptr;
}

blob& blob::operator=(blob&& other)
{
  std::swap(val, other.val);
  std::swap(c_dtor, other.c_dtor);
  return *this;
}

blob::~blob()
{
  if (c_dtor)
    c_dtor(reinterpret_cast<base*>(this));
}
