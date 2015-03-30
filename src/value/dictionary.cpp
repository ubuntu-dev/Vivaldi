#include "dictionary.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::dictionary::dictionary(const std::unordered_map<object*, object*,
                                                       hasher, key_equal>& mems)
  : object {&builtin::type::dictionary, tag::dictionary},
    val    {mems}
{ }
