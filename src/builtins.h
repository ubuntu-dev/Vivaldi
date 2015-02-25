#ifndef VV_BUILTINS_H
#define VV_BUILTINS_H

#include "value.h"
#include "vm/call_frame.h"

namespace vv {

namespace builtin {

namespace sym {

// Defined in builtins.cpp

extern vv::symbol self;
extern vv::symbol call;

}

namespace function {

// Defined in builtins.cpp

extern value::builtin_function print;
extern value::builtin_function puts;
extern value::builtin_function gets;

extern value::builtin_function filter;
extern value::builtin_function map;
extern value::builtin_function reduce;
extern value::builtin_function all;
extern value::builtin_function any;
extern value::builtin_function count;

extern value::builtin_function quit;

}

namespace type {

// Each class has its own file in the builtins/ directory

extern value::type array;
extern value::type array_iterator;
extern value::type boolean;
extern value::type dictionary;
extern value::type custom_type;
extern value::type file;
extern value::type floating_point;
extern value::type function;
extern value::type integer;
extern value::type object;
extern value::type nil;
extern value::type string;
extern value::type string_iterator;
extern value::type symbol;
extern value::type range;

}

void make_base_env(vm::environment& base);

}

}

#endif
