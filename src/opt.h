#ifndef VV_OPT_H
#define VV_OPT_H

#include "expression.h"

namespace vv {

// Call on any chunk of valid (i.e. won't blow up the VM--- it can still result
// in an exception being thrown or whatever) VM code to perform various
// optimizations (e.g constant folding, eliminating unused code, etc.) without
// changing the code's behavior.
void optimize(std::vector<vm::command>& code);
// Call on any complete, independent, piece of code (e.g. a function or the
// contents of a file). Like optimize, but more agressive (e.g. eliminate unused
// variables); calling this on a dependent line of code will probably result in
// errors.
void optimize_independent_block(std::vector<vm::command>& code);

}

#endif
