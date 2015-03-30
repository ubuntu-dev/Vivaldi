#ifndef VV_VALUE_REGEX_H
#define VV_VALUE_REGEX_H

#include "value/object.h"

#include <regex>

namespace vv {

namespace value {

struct regex : public object {
  regex(const std::regex& val = {}, const std::string& str = {});

  std::regex val;
  // Regex in string form (stored for pretty-printing)
  std::string str;
};

struct regex_result : public object {
  regex_result(value::string& str, std::smatch&& res);

  std::smatch val;

  // We're holding on to the owning string, since regex results are stored as
  // pointers into the original string and it'd be a shame if it was deleted
  // before those pointers
  value::string& owning_str;
};

}

}

#endif
