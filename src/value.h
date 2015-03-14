#ifndef VV_VALUE_H
#define VV_VALUE_H

#include "symbol.h"
#include "utils/dumb_ptr.h"
#include "utils/hash_map.h"
#include "utils/vector_ref.h"
#include "vm/instruction.h"

#include <vector>

namespace vv {

// Pre-declarations for headers dependent on this one
namespace ast {
class function_definition;
}

namespace vm {
class machine;
class environment;
}

// Classes representing Vivaldi objects.
namespace value {

struct array;
struct array_iterator;
struct boolean;
struct builtin_function;
struct dictionary;
struct file;
struct floating_point;
struct function;
struct integer;
struct nil;
struct opt_monop;
struct opt_binop;
struct range;
struct regex;
struct regex_result;
struct string;
struct string_iterator;
struct symbol;
struct type;

// Basic Object class from which all types are derived.
struct base {
  // Creates a new value::base of the provided type.
  base(type* type);
  // Creates a new value::base of type Object
  base();

  // Outputs a human-readable string representing the object.
  // The output of base::value is used in the REPL, and in 'puts' and 'print'
  // for non-String types; override in subclasses with something more specific.
  virtual std::string value() const { return "<object>"; }

  virtual ~base() { }

  // Contains local, variable-specific members.
  // Only members of this specific object are stored here; methods are stored
  // inside of their owning classes.
  hash_map<vv::symbol, value::base*> members;
  // Pointer to type (this should never be null. TODO: make reference)
  type* type;

  // Hash method used in dictionaries.
  // Overridden by classes with any useful concept of equality (as a rule of
  // thumb, if a class has an "equals" / method, it should probably override
  // this and base::equals).
  virtual size_t hash() const;
  // Provides an equality function for use in Dictionaries (see base::hash).
  virtual bool equals(const value::base& other) const;

  // Garbage collection interface.

  // Any class that stores references to GCable / objects *must* override
  // mark()--- but make sure you call base::mark in your overridden version!
  virtual void mark();
  // Whether or not this has been marked during this mark/sweep cycle.
  bool marked() const { return m_marked; }
  // Sets marked to false.
  void unmark() { m_marked = false; }

private:
  bool m_marked;
};

// Base class for all callable types. All function types (currently function,
// builtin_function, opt_monop, and opt_binop) are subclasses of basic_function.
struct basic_function : public base {
  // Indicates which type of function this is.
  // Used instead of RTTI and all its convenience because of measurably
  // increased performance.
  enum class func_type {
    opt1,
    opt2,
    builtin,
    vv
  };

  basic_function(func_type type,
                 int argc,
                 vm::environment* enclosing,
                 vector_ref<vm::command> body);

  // The type of function, as enumerated above.
  const func_type type;
  // Expected number of arguments.
  const int argc;
  // Enclosing environment, or nullptr if none exists.
  const dumb_ptr<vm::environment> enclosing;
  // The VM code to run in the new call frame (if this is a C++ function (as in
  // opt_monop, opt_binop, and builtin_function), just provide a stub with a
  // single 'ret false' command.
  vector_ref<vm::command> body;
};

// C++ representation of a Vivaldi Type.
struct type : public base {
  type(const std::function<value::base*()>& constructor,
       const hash_map<vv::symbol, value::base*>& methods,
       value::base& parent,
       vv::symbol name);

  // Class methods.
  // Whenever a object's member is looked for, if it isn't found
  // locally, the class will search its type's methods, and that type's parent's
  // methods, and so on recursively until it's found or there are no more
  // parents left.
  hash_map<vv::symbol, value::base*> methods;

  // Very simple constructor.
  // This constructor should just provides an allocated bit of memory of
  // the appropriate type; any actual initialization (including reading passed
  // arguments) has to happen in the class's "init" method.
  std::function<value::base*()> constructor;

  // Blob of VM code used in initialization.
  // This shim is necessary because, of course, when you create a new object you
  // want to get that object back. Unfortunately it's not possible to guarantee
  // this in the init function, since someone could do something like
  //
  //     class Foo
  //       fn init(): 5
  //     end
  //
  // When you instantiate Foo, you'd prefer you get your new object back, not 5.
  // So the init shim creates a function that looks something like
  //
  //     fn (<args...): do
  //       self.init(<args...>)
  //       self
  //     end
  //
  // and the constructor calls that fake init function instead of 'init'.
  vm::function_t init_shim;

  // Parent class. Parent classes are stored as references, since they're
  // unchangeable and can't ever be null (Object's just points to itself).
  value::base& parent;

  // Name of class. Stored in class so value() can be prettier than just
  // "<type>".
  vv::symbol name;

  // Overriden string representation returning the type's name.
  std::string value() const override;

  void mark() override;
};

}

}

// Call virtual hash() and equals() methods instead of just hashing based on
// pointers
template <>
struct std::hash<vv::value::base*> {
  size_t operator()(const vv::value::base* b) const;
};
template <>
struct std::equal_to<vv::value::base*> {
  bool operator()(const vv::value::base* left, const vv::value::base* right) const;
};

#endif
