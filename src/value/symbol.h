#ifndef VV_VALUE_SYMBOL_H
#define VV_VALUE_SYMBOL_H

#include "../symbol.h"
#include "value/basic_object.h"

namespace vv {

namespace value {

struct symbol : public basic_object {
public:
  symbol(vv::symbol val = {});

  using value_type = vv::symbol;
  value_type value;
};

}

}

#endif
