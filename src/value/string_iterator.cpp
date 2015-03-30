#include "string_iterator.h"

#include "builtins.h"
#include "gc.h"
#include "value/string.h"

using namespace vv;

value::string_iterator::string_iterator(string& new_str)
  : object {&builtin::type::string_iterator, tag::string_iterator},
    str    {new_str},
    idx    {0}
{ }
