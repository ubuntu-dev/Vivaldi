#ifndef VV_AST_MEMBER_H
#define VV_AST_MEMBER_H

#include "expression.h"

namespace vv {

namespace ast {

class member : public expression {
public:
  member(vv::symbol name);

  std::vector<vm::command> generate() const override;

private:
  vv::symbol m_name;
};

}

}

#endif
