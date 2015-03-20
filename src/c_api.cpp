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
std::function<vv_object_ptr_t(Args...)> fn_with_err_check(vv_object_ptr_t(*orig)(Args...))
{
  return [orig](Args&&... args...)
  {
    auto res = orig(std::forward<Args>(args)...);
    if (!res.obj)
      throw_exception("Error calling C extension");
    return res;
  };
}

value::object_ptr to_managed_ptr(vv_object_ptr_t ptr)
{
  return {ptr.obj, *reinterpret_cast<gc::internal::block*>(ptr.internal)};
}

vv_object_ptr_t from_managed_ptr(value::object_ptr ptr)
{
  return {ptr.get(), ptr.has_block() ? &ptr.block() : nullptr};
}

}

extern "C" {

vv_symbol_t vv_make_symbol(const char* string)
{
  return { to_string(symbol{string}).c_str() };
}

int vv_get_int(vv_object_ptr_t obj, int* readinto)
{
  if (obj.obj->type != &builtin::type::integer)
    return -1;
  *readinto = static_cast<value::integer*>(obj.obj)->val;
  return 0;
}

int vv_get_double(vv_object_ptr_t obj, double* readinto)
{
  if (obj.obj->type != &builtin::type::floating_point)
    return -1;
  *readinto = static_cast<value::floating_point*>(obj.obj)->val;
  return 0;
}

int vv_get_bool(vv_object_ptr_t obj, int* readinto)
{
  if (obj.obj->type != &builtin::type::boolean)
    return -1;
  *readinto = static_cast<value::boolean*>(obj.obj)->val;
  return 0;
}

int vv_get_str(vv_object_ptr_t obj, const char** readinto)
{
  if (obj.obj->type != &builtin::type::string)
    return -1;
  *readinto = static_cast<value::string*>(obj.obj)->val.c_str();
  return 0;
}

int vv_get_symbol(vv_object_ptr_t obj, vv_symbol_t* readinto)
{
  if (obj.obj->type != &builtin::type::symbol)
    return -1;
  auto cpp_sym = static_cast<value::symbol*>(obj.obj)->val;
  readinto->string = to_string(cpp_sym).c_str();
  return 0;
}

int vv_get_blob(vv_object_ptr_t obj, void** readinto)
{
  *readinto = static_cast<value::blob*>(obj.obj)->val;
  return 0;
}

int vv_set_blob(vv_object_ptr_t obj, void* blob)
{
  static_cast<value::blob*>(obj.obj)->val = blob;
  return 0;
}

vv_object_ptr_t vv_call_fn(vv_object_ptr_t func, vv_object_ptr_t* args, size_t argc)
{
  for (auto i = argc; i--;)
    cvm().push(to_managed_ptr(args[i]));

  cvm().push(to_managed_ptr(func));
  try {
    cvm().call(argc);
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return from_managed_ptr(nullptr);
  }
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_get_mem(vv_object_ptr_t obj, vv_symbol_t name)
{
  if (obj.obj->members.contains({name.string})) {
    return from_managed_ptr(obj.obj->members.at({name.string}));
  }
  return from_managed_ptr(nullptr);
}

vv_object_ptr_t vv_set_mem(vv_object_ptr_t obj, vv_symbol_t name, vv_object_ptr_t val)
{
  obj.obj->members[{name.string}] = to_managed_ptr(val);
  return val;
}

vv_object_ptr_t vv_get_type(vv_object_ptr_t obj)
{
  return from_managed_ptr(obj.obj->type);
}

vv_object_ptr_t vv_new_object(vv_object_ptr_t type,
                              vv_object_ptr_t* args,
                              size_t argc)
{
  for (size_t i{}; i != argc; ++i)
    cvm().push(to_managed_ptr(args[i]));

  cvm().push(to_managed_ptr(type));
  try {
    cvm().pobj(argc);
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return from_managed_ptr(nullptr);
  }
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_builtin_type_array          ( from_managed_ptr(&builtin::type::array) );
vv_object_ptr_t vv_builtin_type_array_iterator ( from_managed_ptr(&builtin::type::array_iterator) );
vv_object_ptr_t vv_builtin_type_bool           ( from_managed_ptr(&builtin::type::boolean) );
vv_object_ptr_t vv_builtin_type_dictionary     ( from_managed_ptr(&builtin::type::dictionary) );
vv_object_ptr_t vv_builtin_type_file           ( from_managed_ptr(&builtin::type::file) );
vv_object_ptr_t vv_builtin_type_float          ( from_managed_ptr(&builtin::type::floating_point) );
vv_object_ptr_t vv_builtin_type_function       ( from_managed_ptr(&builtin::type::function) );
vv_object_ptr_t vv_builtin_type_int            ( from_managed_ptr(&builtin::type::integer) );
vv_object_ptr_t vv_builtin_type_nil            ( from_managed_ptr(&builtin::type::nil) );
vv_object_ptr_t vv_builtin_type_range          ( from_managed_ptr(&builtin::type::range) );
vv_object_ptr_t vv_builtin_type_regex          ( from_managed_ptr(&builtin::type::regex) );
vv_object_ptr_t vv_builtin_type_string         ( from_managed_ptr(&builtin::type::string) );
vv_object_ptr_t vv_builtin_type_string_iterator( from_managed_ptr(&builtin::type::string_iterator) );
vv_object_ptr_t vv_builtin_type_symbol         ( from_managed_ptr(&builtin::type::symbol) );

vv_object_ptr_t vv_new_bool(int val)
{
  cvm().pbool(val);
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_new_float(double val)
{
  cvm().pflt(val);
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_new_file(const char* filename)
{
  auto file = gc::alloc<value::file>( filename );
  cvm().push(file);
  return from_managed_ptr(file);
}

vv_object_ptr_t vv_new_int(int val)
{
  cvm().pint(val);
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_new_nil()
{
  cvm().pnil();
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_new_regex(const char* reg)
{
  try {
    cvm().pre(reg);
    return from_managed_ptr(cvm().top());
  } catch (const vm_error& e) {
    return from_managed_ptr(nullptr);
  }
}

vv_object_ptr_t vv_new_string(const char* str)
{
  cvm().pstr(str);
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_new_symbol(vv_symbol_t sym)
{
  cvm().psym({sym.string});
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_new_blob(void* blob,
                         void(*dtor)(vv_object_ptr_t),
                         vv_object_ptr_t type)
{
  if (type.obj->type != &builtin::type::custom_type)
    return from_managed_ptr(nullptr);
  auto cpp_dtor = [dtor](value::object_ptr obj) { dtor(from_managed_ptr(obj)); };
  cvm().push(gc::alloc<value::blob>( blob, cpp_dtor ));
  cvm().top()->type = to_managed_ptr(type);
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_alloc_blob(void* blob, void(*dtor)(vv_object_ptr_t))
{
  auto cpp_dtor = [dtor](value::object_ptr obj) { dtor(from_managed_ptr(obj)); };
  return from_managed_ptr(gc::alloc<value::blob>( blob, cpp_dtor ));
}

vv_object_ptr_t vv_get_parent(vv_object_ptr_t type)
{
  if (type.obj->type != &builtin::type::custom_type)
    return from_managed_ptr(nullptr);
  return from_managed_ptr(static_cast<value::type*>(type.obj)->parent);
}

vv_object_ptr_t vv_new_type(const char* name,
                         vv_object_ptr_t parent,
                         vv_object_ptr_t(*ctor)(),
                         vv_object_ptr_t(*init)(vv_object_ptr_t),
                         size_t argc)
{
  auto cpp_parent = parent.obj ? to_managed_ptr(parent) : &builtin::type::object;

  symbol sym_name{name};
  hash_map<symbol, gc::managed_ptr<value::basic_function>> methods{ };
  if (init) {
    auto checked_init = fn_with_err_check(init);
    auto cpp_fn = [checked_init](vm::machine& vm)
    {
      vm.self();
      auto self = vm.top();
      vm.pop(1);
      return to_managed_ptr(checked_init(from_managed_ptr(self)));
    };
    auto fn = gc::alloc<value::builtin_function>(cpp_fn, static_cast<int>(argc));
    methods.insert({"init"}, fn);
    cvm().push(fn);
  }

  auto cpp_ctor = [ctor]{ return to_managed_ptr(ctor()); };

  auto type = gc::alloc<value::type>( cpp_ctor, methods, cpp_parent, sym_name );

  cvm().push(type);
  cvm().let(sym_name);
  return from_managed_ptr(type);
}

vv_object_ptr_t vv_get_arg(size_t argnum)
{
  cvm().arg(static_cast<int>(argnum));
  auto obj = cvm().top();
  cvm().pop(1);
  return from_managed_ptr(obj);
}

vv_object_ptr_t vv_new_function(vv_object_ptr_t(func)(), size_t argc)
{
  auto checked_fn = fn_with_err_check(func);
  auto cpp_func = [checked_fn](vm::machine&)
  {
    return to_managed_ptr(checked_fn());
  };

  auto fn = gc::alloc<value::builtin_function>(cpp_func, static_cast<int>(argc));
  cvm().push(fn);
  return from_managed_ptr(cvm().top());
}

vv_object_ptr_t vv_add_method(vv_object_ptr_t type,
                           const char* name,
                           vv_object_ptr_t(*func)(vv_object_ptr_t),
                           size_t argc)
{
  auto checked_fn = fn_with_err_check(func);
  auto cpp_func = [checked_fn](vm::machine& vm)
  {
    vm.self();
    auto self = vm.top();
    vm.pop(1);
    return to_managed_ptr(checked_fn(from_managed_ptr(self)));
  };

  auto fn = gc::alloc<value::builtin_function>(cpp_func, static_cast<int>(argc));
  static_cast<value::type*>(type.obj)->methods[{name}] = fn;
  return from_managed_ptr(fn);
}

vv_object_ptr_t vv_add_monop(vv_object_ptr_t type,
                          const char* name,
                          vv_object_ptr_t(*func)(vv_object_ptr_t))
{
  auto cpp_func = [func](value::object_ptr obj)
  {
    static auto f = fn_with_err_check(func);
    return to_managed_ptr(f(from_managed_ptr(obj)));
  };
  auto fn = gc::alloc<value::opt_monop>( cpp_func );
  static_cast<value::type*>(type.obj)->methods[{name}] = fn;
  return from_managed_ptr(fn);
}

vv_object_ptr_t vv_add_binop(vv_object_ptr_t type,
                          const char* name,
                          vv_object_ptr_t(*func)(vv_object_ptr_t, vv_object_ptr_t))
{
  auto cpp_func = [func](value::object_ptr first, value::object_ptr second)
  {
    static auto f = fn_with_err_check(func);
    return to_managed_ptr(f(from_managed_ptr(first), from_managed_ptr(second)));
  };
  auto fn = gc::alloc<value::opt_binop>( cpp_func );
  static_cast<value::type*>(type.obj)->methods[{name}] = fn;
  return from_managed_ptr(fn);
}

vv_object_ptr_t vv_let(vv_symbol_t name, vv_object_ptr_t obj)
{
  cvm().push(to_managed_ptr(obj));
  try {
    cvm().let({name.string});
  } catch (const vm_error& e) {
    return from_managed_ptr(nullptr);
  }
  cvm().pop(1);
  return obj;
}

vv_object_ptr_t vv_write(vv_symbol_t name, vv_object_ptr_t obj)
{
  cvm().push(to_managed_ptr(obj));
  try {
    cvm().write({name.string});
  } catch (const vm_error& e) {
    return from_managed_ptr(nullptr);
  }
  cvm().pop(1);
  return obj;
}

vv_object_ptr_t vv_read(vv_symbol_t name)
{
  try {
    cvm().read({name.string});
  } catch (const vm_error& e) {
    return from_managed_ptr(nullptr);
  }
  auto obj = cvm().top();
  cvm().pop(1);
  return from_managed_ptr(obj);
}

}
