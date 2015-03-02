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

  int get_argc() const override { return 0; }
  vector_ref<vm::command> get_body() const override;
  vm::environment* get_enclosing() const override { return nullptr; }

  std::function<base*(base*)> body;
};

struct opt_binop : public basic_function {
public:
  opt_binop(const std::function<base*(base*, base*)>& body);

  std::string value() const override;

  int get_argc() const override { return 1; }
  vector_ref<vm::command> get_body() const override;
  vm::environment* get_enclosing() const override { return nullptr; }

  std::function<base*(base*, base*)> body;
};

}

}

#endif
