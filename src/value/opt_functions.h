#ifndef VV_VALUE_OPT_FUNCTIONS_H
#define VV_VALUE_OPT_FUNCTIONS_H

#include "value.h"
#include "vm.h"

namespace vv {

namespace value {

struct opt_monop : public basic_function {
public:
  opt_monop(const std::function<base*(base*)>& body);

  std::string value() const override;

  std::function<base*(base*)> fn_body;
};

struct opt_binop : public basic_function {
public:
  opt_binop(const std::function<base*(base*, base*)>& body);

  std::string value() const override;

  std::function<base*(base*, base*)> fn_body;
};

}

}

#endif
