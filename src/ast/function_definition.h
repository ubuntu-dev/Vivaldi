#ifndef VV_AST_FUNCTION_DEFINITION_H
#define VV_AST_FUNCTION_DEFINITION_H

#include "expression.h"

#include <boost/optional/optional.hpp>

namespace vv {

namespace ast {

class function_definition : public expression {
public:
  function_definition(symbol name,
                      std::unique_ptr<expression>&& body,
                      const std::vector<symbol>& args,
                      boost::optional<symbol> vararg_name = {});

  std::vector<vm::command> generate() const override;

private:
  symbol m_name;
  std::shared_ptr<expression> m_body;
  std::vector<symbol> m_args;
  boost::optional<symbol> m_vararg_name;
};

}

}

#endif
