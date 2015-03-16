#include "vivaldi.h"

#include "builtins.h"
#include "gc.h"
#include "value.h"
#include "vm.h"
#include "utils/error.h"
#include "value/blob.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/integer.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/symbol.h"

using namespace vv;

namespace {

value::base* to_cpp(vv_object_t* obj)
{
  return reinterpret_cast<value::base*>(obj);
}

vv_object_t* to_c(value::base* obj)
{
  return reinterpret_cast<vv_object_t*>(obj);
}

vm::machine& cvm()
{
  return gc::get_running_vm();
}

}

extern "C" {

vv_symbol_t vv_make_symbol(const char* string)
{
  return { to_string(symbol{string}).c_str() };
}

int vv_get_int(vv_object_t* obj, int* readinto)
{
  if (to_cpp(obj)->type != &builtin::type::integer)
    return -1;
  *readinto = static_cast<value::integer*>(to_cpp(obj))->val;
  return 0;
}

int vv_get_double(vv_object_t* obj, double* readinto)
{
  if (to_cpp(obj)->type != &builtin::type::floating_point)
    return -1;
  *readinto = static_cast<value::floating_point*>(to_cpp(obj))->val;
  return 0;
}

int vv_get_bool(vv_object_t* obj, int* readinto)
{
  if (to_cpp(obj)->type != &builtin::type::boolean)
    return -1;
  *readinto = static_cast<value::boolean*>(to_cpp(obj))->val;
  return 0;
}

int vv_get_str(vv_object_t* obj, const char** readinto)
{
  if (to_cpp(obj)->type != &builtin::type::string)
    return -1;
  *readinto = static_cast<value::string*>(to_cpp(obj))->val.c_str();
  return 0;
}

int vv_get_symbol(vv_object_t* obj, vv_symbol_t* readinto)
{
  if (to_cpp(obj)->type != &builtin::type::symbol)
    return -1;
  auto cpp_sym = static_cast<value::symbol*>(to_cpp(obj))->val;
  readinto->string = to_string(cpp_sym).c_str();
  return 0;
}

int vv_get_blob(vv_object_t* obj, void** readinto)
{
  *readinto = static_cast<value::blob*>(to_cpp(obj))->val;
  return 0;
}

int vv_set_blob(vv_object_t* obj, void* blob)
{
  static_cast<value::blob*>(to_cpp(obj))->val = blob;
  return 0;
}

vv_object_t* vv_call_fn(vv_object_t* func, vv_object_t** args, size_t argc)
{
  for (auto i = argc; i--;)
    cvm().push(to_cpp(args[i]));

  cvm().push(to_cpp(func));
  try {
    cvm().call(argc);
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return nullptr;
  }
  return to_c(cvm().top());
}

vv_object_t* vv_get_mem(vv_object_t* obj, vv_symbol_t name)
{
  if (to_cpp(obj)->members.contains({name.string})) {
    return to_c(to_cpp(obj)->members.at({name.string}));
  }
  return nullptr;
}

vv_object_t* vv_set_mem(vv_object_t* obj, vv_symbol_t name, vv_object_t* val)
{
  to_cpp(obj)->members[{name.string}] = to_cpp(val);
  return val;
}

vv_object_t* vv_get_type(vv_object_t* obj)
{
  return to_c(to_cpp(obj)->type);
}

vv_object_t* vv_create_object(vv_object_t* type, vv_object_t** args, size_t argc)
{
  for (auto i = argc; i--;)
    cvm().push(to_cpp(args[i]));

  cvm().push(to_cpp(type));
  try {
    cvm().pobj(argc);
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return nullptr;
  }
  return to_c(cvm().top());
}

vv_object_t* vv_builtin_type_array          {to_c(&builtin::type::array)};
vv_object_t* vv_builtin_type_array_iterator {to_c(&builtin::type::array_iterator)};
vv_object_t* vv_builtin_type_bool           {to_c(&builtin::type::boolean)};
vv_object_t* vv_builtin_type_dictionary     {to_c(&builtin::type::dictionary)};
vv_object_t* vv_builtin_type_file           {to_c(&builtin::type::file)};
vv_object_t* vv_builtin_type_float          {to_c(&builtin::type::floating_point)};
vv_object_t* vv_builtin_type_function       {to_c(&builtin::type::function)};
vv_object_t* vv_builtin_type_int            {to_c(&builtin::type::integer)};
vv_object_t* vv_builtin_type_nil            {to_c(&builtin::type::nil)};
vv_object_t* vv_builtin_type_range          {to_c(&builtin::type::range)};
vv_object_t* vv_builtin_type_regex          {to_c(&builtin::type::regex)};
vv_object_t* vv_builtin_type_string         {to_c(&builtin::type::string)};
vv_object_t* vv_builtin_type_string_iterator{to_c(&builtin::type::string_iterator)};
vv_object_t* vv_builtin_type_symbol         {to_c(&builtin::type::symbol)};

vv_object_t* vv_create_bool(int val)
{
  cvm().pbool(val);
  return to_c(cvm().top());
}

vv_object_t* vv_create_float(double val)
{
  cvm().pflt(val);
  return to_c(cvm().top());
}

vv_object_t* vv_create_file(const char* filename)
{
  auto file = gc::alloc<value::file>( filename );
  cvm().push(file);
  return to_c(file);
}

vv_object_t* vv_create_int(int val)
{
  cvm().pint(val);
  return to_c(cvm().top());
}

vv_object_t* vv_create_nil()
{
  cvm().pnil();
  return to_c(cvm().top());
}

vv_object_t* vv_create_regex(const char* reg)
{
  try {
    cvm().pre(reg);
    return to_c(cvm().top());
  } catch (const vm_error& e) {
    return nullptr;
  }
}

vv_object_t* vv_create_string(const char* str)
{
  cvm().pstr(str);
  return to_c(cvm().top());
}

vv_object_t* vv_create_symbol(vv_symbol_t sym)
{
  cvm().psym({sym.string});
  return to_c(cvm().top());
}

vv_object_t* vv_create_blob(void* blob, void(*dtor)(vv_object_t*))
{
  cvm().push(gc::alloc<value::blob>( blob, dtor ));
  return to_c(cvm().top());
}

vv_object_t* vv_get_parent(vv_object_t* type)
{
  if (to_cpp(type)->type != &builtin::type::custom_type)
    return nullptr;
  return to_c(&static_cast<value::type*>(to_cpp(type))->parent);
}

vv_object_t* vv_create_type(const char* name,
                           vv_object_t* parent,
                           vv_object_t*(*ctor)())
{
  auto& cpp_parent = parent ? static_cast<value::type&>(*to_cpp(parent))
                            : builtin::type::object;

  symbol sym_name{name};
  auto type = gc::alloc<value::type>(
      [ctor] { return to_cpp(ctor()); },
      hash_map<symbol, value::base*>{ },
      cpp_parent,
      sym_name );

  cvm().push(type);
  cvm().let(sym_name);
  return to_c(type);
}

vv_object_t* vv_get_arg(size_t argnum)
{
  cvm().arg(static_cast<int>(argnum));
  auto* obj = to_c(cvm().top());
  cvm().pop(1);
  return obj;
}

vv_object_t* vv_create_function(vv_object_t*(func)(), size_t argc)
{
  auto cpp_func = [func](vm::machine&) { return to_cpp(func()); };

  auto fn = gc::alloc<value::builtin_function>( cpp_func,
                                                        static_cast<int>(argc) );
  cvm().push(fn);
  return to_c(cvm().top());
}

vv_object_t* vv_define_method(vv_object_t* type,
                             const char* name,
                             vv_object_t*(*func)(vv_object_t*),
                             size_t argc)
{
  auto cpp_func = [func](vm::machine& vm)
  {
    vm.self();
    auto* self = vm.top();
    vm.pop(1);
    return to_cpp(func(to_c(self)));
  };

  auto fn = gc::alloc<value::builtin_function>( cpp_func,
                                                        static_cast<int>(argc) );
  static_cast<value::type*>(to_cpp(type))->methods[{name}] = fn;
  return to_c(fn);
}

vv_object_t* vv_define_monop(vv_object_t* type,
                            const char* name,
                            vv_object_t*(*func)(vv_object_t*))
{
  auto cpp_func = [func](value::base* self)
                        { return to_cpp(func(to_c(self))); };
  auto fn = gc::alloc<value::opt_monop>( cpp_func );
  static_cast<value::type*>(to_cpp(type))->methods[{name}] = fn;
  return to_c(fn);
}

vv_object_t* vv_define_binop(vv_object_t* type,
                            const char* name,
                            vv_object_t*(*func)(vv_object_t*, vv_object_t*))
{
  auto cpp_func = [func](value::base* self, value::base* arg)
                        { return to_cpp(func(to_c(self), to_c(arg))); };
  auto fn = gc::alloc<value::opt_binop>( cpp_func );
  static_cast<value::type*>(to_cpp(type))->methods[{name}] = fn;
  return to_c(fn);
}

vv_object_t* vv_let(vv_symbol_t name, vv_object_t* obj)
{
  cvm().push(to_cpp(obj));
  try {
    cvm().let({name.string});
  } catch (const vm_error& e) {
    return nullptr;
  }
  cvm().pop(1);
  return obj;
}

vv_object_t* vv_write(vv_symbol_t name, vv_object_t* obj)
{
  cvm().push(to_cpp(obj));
  try {
    cvm().write({name.string});
  } catch (const vm_error& e) {
    return nullptr;
  }
  cvm().pop(1);
  return obj;
}

vv_object_t* vv_read(vv_symbol_t name)
{
  try {
    cvm().read({name.string});
  } catch (const vm_error& e) {
    return nullptr;
  }
  auto* obj = to_c(cvm().top());
  cvm().pop(1);
  return obj;
}

}
