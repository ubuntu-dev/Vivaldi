#include "vivaldi.h"

#include "builtins.h"
#include "gc.h"
#include "value.h"
#include "vm.h"
#include "gc/alloc.h"
#include "utils/error.h"
#include "utils/lang.h"
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

vm::machine& cvm()
{
  return gc::get_running_vm();
}

template <typename... Args>
std::function<vv_object_t*(Args...)> fn_with_err_check(vv_object_t*(*orig)(Args...))
{
  return [orig](Args&&... args...)
  {
    auto res = orig(std::forward<Args>(args)...);
    if (!res)
      throw_exception("Error calling C extension");
    return res;
  };
}

}

extern "C" {

vv_symbol_t vv_make_symbol(const char* string)
{
  return { to_string(symbol{string}).c_str() };
}

int vv_get_int(vv_object_t* obj, int* readinto)
{
  if (obj->type != &builtin::type::integer)
    return -1;
  *readinto = static_cast<value::integer*>(obj)->val;
  return 0;
}

int vv_get_double(vv_object_t* obj, double* readinto)
{
  if (obj->type != &builtin::type::floating_point)
    return -1;
  *readinto = static_cast<value::floating_point*>(obj)->val;
  return 0;
}

int vv_get_bool(vv_object_t* obj, int* readinto)
{
  if (obj->type != &builtin::type::boolean)
    return -1;
  *readinto = static_cast<value::boolean*>(obj)->val;
  return 0;
}

int vv_get_str(vv_object_t* obj, const char** readinto)
{
  if (obj->type != &builtin::type::string)
    return -1;
  *readinto = static_cast<value::string*>(obj)->val.c_str();
  return 0;
}

int vv_get_symbol(vv_object_t* obj, vv_symbol_t* readinto)
{
  if (obj->type != &builtin::type::symbol)
    return -1;
  auto cpp_sym = static_cast<value::symbol*>(obj)->val;
  readinto->string = to_string(cpp_sym).c_str();
  return 0;
}

int vv_get_blob(vv_object_t* obj, void** readinto)
{
  *readinto = static_cast<value::blob*>(obj)->val;
  return 0;
}

int vv_set_blob(vv_object_t* obj, void* blob)
{
  static_cast<value::blob*>(obj)->val = blob;
  return 0;
}

vv_object_t* vv_call_fn(vv_object_t* func, vv_object_t** args, size_t argc)
{
  for (auto i = argc; i--;)
    cvm().push(args[i]);

  cvm().push(func);
  try {
    cvm().call(argc);
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return nullptr;
  }
  return cvm().top();
}

vv_object_t* vv_get_mem(vv_object_t* obj, vv_symbol_t name)
{
  if (obj->members.contains({name.string})) {
    return obj->members.at({name.string});
  }
  return nullptr;
}

vv_object_t* vv_set_mem(vv_object_t* obj, vv_symbol_t name, vv_object_t* val)
{
  obj->members[{name.string}] = val;
  return val;
}

vv_object_t* vv_get_type(vv_object_t* obj)
{
  return obj->type;
}

vv_object_t* vv_new_object(vv_object_t* type,
                              vv_object_t** args,
                              size_t argc)
{
  for (size_t i{}; i != argc; ++i)
    cvm().push(args[i]);

  cvm().push(type);
  try {
    cvm().pobj(argc);
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return nullptr;
  }
  return cvm().top();
}

vv_object_t* vv_builtin_type_array          {&builtin::type::array};
vv_object_t* vv_builtin_type_array_iterator {&builtin::type::array_iterator};
vv_object_t* vv_builtin_type_bool           {&builtin::type::boolean};
vv_object_t* vv_builtin_type_dictionary     {&builtin::type::dictionary};
vv_object_t* vv_builtin_type_file           {&builtin::type::file};
vv_object_t* vv_builtin_type_float          {&builtin::type::floating_point};
vv_object_t* vv_builtin_type_function       {&builtin::type::function};
vv_object_t* vv_builtin_type_int            {&builtin::type::integer};
vv_object_t* vv_builtin_type_nil            {&builtin::type::nil};
vv_object_t* vv_builtin_type_range          {&builtin::type::range};
vv_object_t* vv_builtin_type_regex          {&builtin::type::regex};
vv_object_t* vv_builtin_type_string         {&builtin::type::string};
vv_object_t* vv_builtin_type_string_iterator{&builtin::type::string_iterator};
vv_object_t* vv_builtin_type_symbol         {&builtin::type::symbol};

vv_object_t* vv_new_bool(int val)
{
  cvm().pbool(val);
  return cvm().top();
}

vv_object_t* vv_new_float(double val)
{
  cvm().pflt(val);
  return cvm().top();
}

vv_object_t* vv_new_file(const char* filename)
{
  auto file = gc::alloc<value::file>( filename );
  cvm().push(file);
  return file;
}

vv_object_t* vv_new_int(int val)
{
  cvm().pint(val);
  return cvm().top();
}

vv_object_t* vv_new_nil()
{
  cvm().pnil();
  return cvm().top();
}

vv_object_t* vv_new_regex(const char* reg)
{
  try {
    cvm().pre(reg);
    return cvm().top();
  } catch (const vm_error& e) {
    return nullptr;
  }
}

vv_object_t* vv_new_string(const char* str)
{
  cvm().pstr(str);
  return cvm().top();
}

vv_object_t* vv_new_symbol(vv_symbol_t sym)
{
  cvm().psym({sym.string});
  return cvm().top();
}

vv_object_t* vv_new_blob(void* blob,
                         void(*dtor)(vv_object_t*),
                         vv_object_t* type)
{
  if (type->type != &builtin::type::custom_type)
    return nullptr;
  cvm().push(gc::alloc<value::blob>( blob, dtor ));
  cvm().top()->type = static_cast<value::type*>(type);
  return cvm().top();
}

vv_object_t* vv_alloc_blob(void* blob, void(*dtor)(vv_object_t*))
{
  return gc::alloc<value::blob>( blob, dtor );
}

vv_object_t* vv_get_parent(vv_object_t* type)
{
  if (type->type != &builtin::type::custom_type)
    return nullptr;
  return &static_cast<value::type*>(type)->parent;
}

vv_object_t* vv_new_type(const char* name,
                         vv_object_t* parent,
                         vv_object_t*(*ctor)(),
                         vv_object_t*(*init)(vv_object_t*),
                         size_t argc)
{
  auto& cpp_parent = parent ? static_cast<value::type&>(*parent)
                            : builtin::type::object;

  symbol sym_name{name};
  hash_map<symbol, value::basic_function*> methods{ };
  if (init) {
    auto checked_init = fn_with_err_check(init);
    auto cpp_fn = [checked_init](vm::machine& vm)
    {
      vm.self();
      auto self = vm.top();
      vm.pop(1);
      return checked_init(self);
    };
    auto fn = gc::alloc<value::builtin_function>(cpp_fn, static_cast<int>(argc));
    methods.insert({"init"}, fn);
    cvm().push(fn);
  }

  auto type = gc::alloc<value::type>( ctor, methods, cpp_parent, sym_name );

  cvm().push(type);
  cvm().let(sym_name);
  return type;
}

vv_object_t* vv_get_arg(size_t argnum)
{
  cvm().arg(static_cast<int>(argnum));
  auto obj = cvm().top();
  cvm().pop(1);
  return obj;
}

vv_object_t* vv_new_function(vv_object_t*(func)(), size_t argc)
{
  auto checked_fn = fn_with_err_check(func);
  auto cpp_func = [checked_fn](vm::machine&)
  {
    return checked_fn();
  };

  auto fn = gc::alloc<value::builtin_function>(cpp_func, static_cast<int>(argc));
  cvm().push(fn);
  return cvm().top();
}

vv_object_t* vv_add_method(vv_object_t* type,
                           const char* name,
                           vv_object_t*(*func)(vv_object_t*),
                           size_t argc)
{
  auto checked_fn = fn_with_err_check(func);
  auto cpp_func = [checked_fn](vm::machine& vm)
  {
    vm.self();
    auto self = vm.top();
    vm.pop(1);
    return checked_fn(self);
  };

  auto fn = gc::alloc<value::builtin_function>(cpp_func, static_cast<int>(argc));
  static_cast<value::type*>(type)->methods[{name}] = fn;
  return fn;
}

vv_object_t* vv_add_monop(vv_object_t* type,
                          const char* name,
                          vv_object_t*(*func)(vv_object_t*))
{
  auto fn = gc::alloc<value::opt_monop>( fn_with_err_check(func) );
  static_cast<value::type*>(type)->methods[{name}] = fn;
  return fn;
}

vv_object_t* vv_add_binop(vv_object_t* type,
                          const char* name,
                          vv_object_t*(*func)(vv_object_t*, vv_object_t*))
{
  auto fn = gc::alloc<value::opt_binop>( fn_with_err_check(func) );
  static_cast<value::type*>(type)->methods[{name}] = fn;
  return fn;
}

vv_object_t* vv_let(vv_symbol_t name, vv_object_t* obj)
{
  cvm().push(obj);
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
  cvm().push(obj);
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
  auto obj = cvm().top();
  cvm().pop(1);
  return obj;
}

}
