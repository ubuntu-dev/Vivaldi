#ifndef VV_AST_METHOD_H
#define VV_AST_METHOD_H

#include "expression.h"

namespace vv {

namespace ast {

class method : public expression {
public:
  method(std::unique_ptr<ast::expression>&& object, vv::symbol name);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<ast::expression> m_object;
  vv::symbol m_name;
};

}

}

#endif
