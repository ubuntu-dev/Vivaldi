#ifndef VV_REPL_H
#define VV_REPL_H

#include "expression.h"

namespace vv {

std::vector<std::unique_ptr<ast::expression>> get_valid_line();
void run_repl();

}

#endif
