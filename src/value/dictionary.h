#ifndef VV_VALUE_DICTIONARY_H
#define VV_VALUE_DICTIONARY_H

#include "value/object.h"

#include <unordered_map>

namespace vv {

namespace value {

struct dictionary : public object {
public:
  struct hasher {
    size_t operator()(object* obj) const { return hash_for(*obj); }
  };
  struct key_equal {
    size_t operator()(object* first, object* second) const
    {
      return equals(*first, *second);
    }
  };

  dictionary(const std::unordered_map<object*, object*, hasher, key_equal>& mems = {});

  std::unordered_map<object*, object*, hasher, key_equal> val;
};

}

}

#endif
