#include "string_iterator.h"

#include "builtins.h"

using namespace vv;

value::string_iterator::string_iterator(string& new_str)
  : basic_object {&builtin::type::string_iterator, tag::string_iterator},
    str          {new_str},
    idx          {0}
{ }
