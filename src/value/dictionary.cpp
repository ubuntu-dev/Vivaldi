#include "dictionary.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::dictionary::dictionary(const std::unordered_map<basic_object*, basic_object*,
                                                       hasher, key_equal>& mems)
  : basic_object {&builtin::type::dictionary, tag::dictionary},
    val          {mems}
{ }
