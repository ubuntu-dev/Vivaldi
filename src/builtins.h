#ifndef VV_BUILTINS_H
#define VV_BUILTINS_H


namespace vv {

class symbol;

namespace gc {

class managed_ptr;

}

namespace builtin {

// Commonly used symbols; defined for convenience and to prevent repeated
// constructions, since constructing a symbol is relatively expensive.
namespace sym {

// Defined in builtins.cpp

const extern vv::symbol self;
const extern vv::symbol call;

const extern vv::symbol start;
const extern vv::symbol at_end;
const extern vv::symbol get;
const extern vv::symbol increment;

const extern vv::symbol add;
const extern vv::symbol subtract;
const extern vv::symbol times;
const extern vv::symbol divides;

const extern vv::symbol equals;
const extern vv::symbol unequal;

const extern vv::symbol greater;
const extern vv::symbol less;
const extern vv::symbol greater_equals;
const extern vv::symbol less_equals;
}

// Standalone functions in the standard library.
namespace function {

// Defined in builtins.cpp

extern gc::managed_ptr print;
extern gc::managed_ptr puts;
extern gc::managed_ptr gets;

extern gc::managed_ptr filter;
extern gc::managed_ptr map;
extern gc::managed_ptr reduce;
extern gc::managed_ptr sort;
extern gc::managed_ptr all;
extern gc::managed_ptr any;
extern gc::managed_ptr count;

extern gc::managed_ptr quit;
extern gc::managed_ptr reverse;

}

// Types in the standard library.
namespace type {

// Each class has its own file in the builtins/ directory

extern gc::managed_ptr array;
extern gc::managed_ptr array_iterator;
extern gc::managed_ptr boolean;
extern gc::managed_ptr dictionary;
extern gc::managed_ptr custom_type;
extern gc::managed_ptr file;
extern gc::managed_ptr floating_point;
extern gc::managed_ptr function;
extern gc::managed_ptr integer;
extern gc::managed_ptr object;
extern gc::managed_ptr nil;
extern gc::managed_ptr range;
extern gc::managed_ptr regex;
extern gc::managed_ptr regex_result;
extern gc::managed_ptr string;
extern gc::managed_ptr string_iterator;
extern gc::managed_ptr symbol;

}

void init();

// Populate the provided environment with standard library types and functions.
void make_base_env(gc::managed_ptr base);

}

}

#endif
