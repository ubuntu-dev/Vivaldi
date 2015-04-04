#ifndef VV_VALUE_DICTIONARY_H
#define VV_VALUE_DICTIONARY_H

#include "value/basic_object.h"

#include <unordered_map>

namespace vv {

namespace value {

struct dictionary : public basic_object {
public:
  struct hasher {
    size_t operator()(gc::managed_ptr obj) const { return hash_for(obj); }
  };
  struct key_equal {
    bool operator()(gc::managed_ptr lhs, gc::managed_ptr rhs) const
    {
      return equals(lhs, rhs);
    }
  };

  using value_type = std::unordered_map<gc::managed_ptr, gc::managed_ptr,
                                        hasher, key_equal>;

  dictionary(const value_type& val = {});

  value_type value;
};

}

}

#endif
