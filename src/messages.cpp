#include "messages.h"

#include "utils/lang.h"
#include "value/type.h"

#include <sstream>

using namespace vv;

const std::string message::inheritance_type_err{
  "Types can only inherit from other Types"
};

const std::string message::construction_type_err{
  "Objects can only be constructed from Types"
};

const std::string message::invalid_self_access{
  "self cannot be accessed outside of of objects"
};

const std::string message::divide_by_zero{"Cannot divide by zero"};

std::string message::invalid_regex(const std::string& error)
{
  return "Invalid regex: " + error;
}

std::string message::no_such_variable(vv::symbol var)
{
  return "No such variable: " + to_string(var);
}

std::string message::already_exists(vv::symbol var)
{
  return "Variable " + to_string(var) += " already exists";
}

std::string message::has_no_member(gc::managed_ptr obj, vv::symbol mem)
{
  return value_for(obj) += " has no member " + to_string(mem);
}

std::string message::not_callable(gc::managed_ptr callee)
{
  return "Object " + value_for(callee) += " of type " +
         value_for(callee.type()) += " cannot be called";
}

std::string message::wrong_argc(int expected, int recieved)
{
  return "Wrong number of arguments--- expected " + std::to_string(expected) +=
         ", got " + std::to_string(recieved);
}

std::string message::nonconstructible(gc::managed_ptr type)
{
  return "Objects of type " + value_for(type) += " cannot be directly construced";
}

std::string message::init_type_error(gc::managed_ptr self,
                                     gc::managed_ptr expected,
                                     gc::managed_ptr recieved)
{
  return "objects of type " + value_for(self) +=
         " can only be constructed from objects of type " +
         value_for(expected) += ", not " + value_for(recieved);
}

std::string message::init_multi_type_error(gc::managed_ptr self,
                                           gc::managed_ptr recieved)
{
  return "Objects of type " + value_for(self) +=
         " cannot be constructed from objects of type " + value_for(recieved);
}

std::string message::add_type_error(gc::managed_ptr self,
                                    gc::managed_ptr expected)
{
  return "Only objects of type " + value_for(expected) +=
         " can be added to objects of type " + value_for(self);
}

std::string message::at_type_error(gc::managed_ptr self,
                                   gc::managed_ptr expected)
{
  return "Only objects of type " + value_for(expected) +=
         " can index into objects of type " + value_for(self);
}

std::string message::type_error(gc::managed_ptr expected,
                                gc::managed_ptr recieved)
{
  return "Function expected " + value_for(expected) += ", but recieved " +
         value_for(recieved);
}

std::string message::iterator_owner_error(gc::managed_ptr owner)
{
  return "Expected iterators from the same " + value_for(owner);
}

std::string message::iterator_past_start(gc::managed_ptr self)
{
  return value_for(self) += " cannot be decremented past start";
}

std::string message::iterator_past_end(gc::managed_ptr self)
{
  return value_for(self) += " cannot be incremented past end";
}

std::string message::iterator_at_end(gc::managed_ptr self)
{
  return value_for(self) += " cannot be accessed at end";
}

std::string message::out_of_range(size_t lower, size_t upper, int recieved)
{
  std::ostringstream sstm;
  sstm << "Out of range (expected " << lower << '-' << upper << ", got "
       << recieved << ')';
  return sstm.str();
}

std::string message::caught_exception(gc::managed_ptr err)
{
  return "Caught exception: " + value_for(err);
}
