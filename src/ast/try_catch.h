#ifndef VV_AST_TRY_CATCH_H
#define VV_AST_TRY_CATCH_H

#include "expression.h"

namespace vv {

namespace ast {

struct catch_stmt {
  symbol exception_name;
  symbol exception_type;
  std::unique_ptr<expression> catcher;
};

class try_catch : public expression {
public:
  try_catch(std::unique_ptr<expression>&& body,
            std::vector<catch_stmt>&& catchers);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_body;
  std::vector<catch_stmt> m_catchers;
};

}

}

#endif
