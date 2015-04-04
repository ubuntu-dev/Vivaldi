#ifndef VV_VALUE_REGEX_H
#define VV_VALUE_REGEX_H

#include "value/basic_object.h"

#include <regex>

namespace vv {

namespace value {

struct regex : public basic_object {
  regex(const std::regex& val = {}, const std::string& str = {});

  struct value_type {
    std::regex val;
    // Regex in string form (stored for pretty-printing)
    std::string str;
  };

  value_type value;
};

struct regex_result : public basic_object {
  regex_result(gc::managed_ptr str, std::smatch&& res);

  struct value_type {
    std::smatch val;

    // We're holding on to the owning string, since regex results are stored as
    // pointers into the original string and it'd be a shame if it was deleted
    // before those pointers
    gc::managed_ptr owning_str;
  };

  value_type value;
};

}

}

#endif
