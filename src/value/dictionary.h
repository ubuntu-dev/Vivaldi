#ifndef VV_VALUE_DICTIONARY_H
#define VV_VALUE_DICTIONARY_H

#include "value/basic_object.h"

#include <unordered_map>

namespace vv {

namespace value {

struct dictionary : public basic_object {
public:
  struct hasher {
    size_t operator()(basic_object* obj) const { return hash_for(*obj); }
  };
  struct key_equal {
    size_t operator()(basic_object* lhs, basic_object* rhs) const
    {
      return equals(*lhs, *rhs);
    }
  };

  dictionary(const std::unordered_map<basic_object*, basic_object*,
                                      hasher, key_equal>& mems = {});

  std::unordered_map<basic_object*, basic_object*, hasher, key_equal> val;
};

}

}

#endif
