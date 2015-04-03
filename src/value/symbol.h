#ifndef VV_VALUE_SYMBOL_H
#define VV_VALUE_SYMBOL_H

#include "../symbol.h"
#include "value/basic_object.h"

namespace vv {

namespace value {

struct symbol : public basic_object {
public:
  symbol(vv::symbol val = {});

  vv::symbol val;
};

}

}

#endif
