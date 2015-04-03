#include "blob.h"

#include "builtins.h"

using namespace vv;
using namespace value;

blob::blob(void* val, const std::function<void(basic_object*)>& dtor)
  : basic_object {&builtin::type::object, tag::blob},
    val          {val},
    c_dtor       {dtor}
{ }

blob::blob(blob&& other)
  : basic_object {&builtin::type::object, tag::blob},
    val          {other.val},
    c_dtor       {std::move(other.c_dtor)}
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
    c_dtor(reinterpret_cast<basic_object*>(this));
}
