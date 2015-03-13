#ifndef VV_VALUE_REGEX_H
#define VV_VALUE_REGEX_H

#include "value.h"

#include <regex>

namespace vv {

namespace value {

struct regex : public base {
public:
  regex(const std::regex& val = {});

  std::string value() const override;

  std::regex val;
};

struct regex_result : public base {
public:
  regex_result(value::string& str, std::smatch&& res);

  std::string value() const override;

  void mark() override;

  std::smatch val;

  value::string& owning_str;
private:
};

}

}

#endif
