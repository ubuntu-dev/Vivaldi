#ifndef VV_MESSAGE_H
#define VV_MESSAGE_H

#include "value.h"
#include "symbol.h"

// Predefined error messages, for use in exceptions, REPL messages, etc.

namespace vv {

namespace message {

// attempted to inherit from something other than a type
const extern std::string inheritance_type_err;
// attempted to call 'new' on something other than a type
const extern std::string construction_type_err;
// attempted to call self outside of an basic_object
const extern std::string invalid_self_access;
const extern std::string divide_by_zero;

// Invalid regex
std::string invalid_regex(const std::string& error);
// Attempted to read/write to a nonexistent variable
std::string no_such_variable(vv::symbol var);
// Attempted to declare an extant variable
std::string already_exists(vv::symbol var);
// Attempted to read an basic_object member that doesn't exist
std::string has_no_member(gc::managed_ptr obj, vv::symbol mem);
// Attempted to call something other than a function
std::string not_callable(gc::managed_ptr callee);
// Function called with the wrong number of arguments
std::string wrong_argc(int expected, int recieved);
// Attempt to directly construct a nonconstructible basic_object (e.g. Integer)
std::string nonconstructible(gc::managed_ptr type);

// Constructor called with wrong argument type
std::string init_type_error(gc::managed_ptr self,
                            gc::managed_ptr expected,
                            gc::managed_ptr recieved);
// Constructor called with wrong argument type, if more than one type is
// permissible
std::string init_multi_type_error(gc::managed_ptr self,
                                  gc::managed_ptr recieved);
// Addition type error
std::string add_type_error(gc::managed_ptr self,
                           gc::managed_ptr expected);
// Indexing type error
std::string at_type_error(gc::managed_ptr self, gc::managed_ptr expected);
// Generic type error
std::string type_error(gc::managed_ptr expected, gc::managed_ptr recieved);

// Use if an operation on two iterators from the same owner is called on two
// iterators from different owners (e.g '[].start() - [].start')
std::string iterator_owner_error(gc::managed_ptr owner);
// Use if an iterator is decremented past start
std::string iterator_past_start(gc::managed_ptr self);
// Use if an iterator is incremented past end
std::string iterator_past_end(gc::managed_ptr self);
// Use if an iterator is dereferenced at end
std::string iterator_at_end(gc::managed_ptr self);

// A value (integer, generally) is of the correct type but not within the
// correct bounds (for instance, 11894530.chr(), or ['foo][1024])
std::string out_of_range(size_t lower, size_t upper, int recieved);

// Error message for unhandled exception
std::string caught_exception(gc::managed_ptr error);

}

}

#endif
