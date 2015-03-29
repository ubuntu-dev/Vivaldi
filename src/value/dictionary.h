#ifndef VV_VALUE_DICTIONARY_H
#define VV_VALUE_DICTIONARY_H

#include "value.h"

namespace vv {

namespace value {

struct dictionary : public object {
public:
  struct hasher {
    size_t operator()(object* obj) const { return obj->hash(); }
  };
  struct key_equal {
    size_t operator()(object* first, object* second) const
    {
      return first->equals(*second);
    }
  };

  dictionary(const std::unordered_map<object*, object*, hasher, key_equal>& mems = {});

  std::string value() const override;
  void mark() override;

  std::unordered_map<object*, object*, hasher, key_equal> val;
};

}

}

#endif
