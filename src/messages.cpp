#include "messages.h"

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

std::string message::has_no_member(const value::base& obj, vv::symbol mem)
{
  return obj.value() += " has no member " + to_string(mem);
}

std::string message::not_callable(const value::base& callee)
{
  return "Object " + callee.value() += " of type " + callee.type->value() +=
         " cannot be called";
}

std::string message::wrong_argc(int expected, int recieved)
{
  return "Wrong number of arguments--- expected " + std::to_string(expected) +=
         ", got " + std::to_string(recieved);
}

std::string message::nonconstructible(const value::type& type)
{
  return "Objects of type " + type.value() += " cannot be directly construced";
}

std::string message::init_type_error(const value::type& self,
                                     const value::type& expected,
                                     const value::type& recieved)
{
  return "Objects of type " + self.value() +=
         " can only be constructed from objects of type " + expected.value() +=
         ", not " + recieved.value();
}

std::string message::add_type_error(const value::type& self,
                                    const value::type& expected)
{
  return "Only objects of type " + expected.value() +=
         " can be added to objects of type " + self.value();
}

std::string message::at_type_error(const value::type& self,
                                   const value::type& expected)
{
  return "Only objects of type " + expected.value() +=
         " can index into objects of type " + self.value();
}

std::string message::type_error(const value::type& expected,
                                const value::type& recieved)
{
  return "Function expected " + expected.value() += ", but recieved " +
         recieved.value();
}

std::string message::iterator_owner_error(const value::type& owner)
{
  return "Expected iterators from the same " + owner.value();
}

std::string message::iterator_past_start(const value::type& self)
{
  return self.value() += " cannot be decremented past start";
}

std::string message::iterator_past_end(const value::type& self)
{
  return self.value() += " cannot be decremented past end";
}

std::string message::iterator_at_end(const value::type& self)
{
  return self.value() += " cannot be accessed at end";
}

std::string message::out_of_range(size_t lower, size_t upper, int recieved)
{
  std::ostringstream sstm;
  sstm << "Out of range (expected " << lower << '-' << upper << ", got "
       << recieved << ')';
  return sstm.str();
}

std::string message::caught_exception(const value::base& err)
{
  return "Caught execption: " + err.value();
}
