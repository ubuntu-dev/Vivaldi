#include "vivaldi.h"
#include "c_internal.h"

#include "builtins.h"
#include "gc.h"
#include "value.h"
#include "vm.h"
#include "gc/alloc.h"
#include "utils/error.h"
#include "utils/lang.h"
#include "value/blob.h"
#include "value/builtin_function.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/symbol.h"
#include "value/type.h"

using namespace vv;

namespace {

vm::machine& cvm()
{
  return gc::get_running_vm();
}

template <typename... Args>
std::function<vv_object_t(Args...)> fn_with_err_check(vv_object_t(*orig)(Args...))
{
  return [orig](Args&&... args)
  {
    const auto res = orig(std::forward<Args>(args)...);
    if (!res)
      throw_exception(builtin::type::runtime_error,
                      "Error calling C extension");
    return res;
  };
}

}

gc::managed_ptr vv::cast_from(vv_object_t obj)
{
  return *reinterpret_cast<gc::managed_ptr*>(&obj);
}

vv_object_t vv::cast_to(gc::managed_ptr ptr)
{
  return *reinterpret_cast<vv_object_t*>(&ptr);
}

extern "C" {

const vv_object_t vv_null{cast_to({})};

vv_symbol_t vv_make_symbol(const char* string)
{
  return { to_string(symbol{string}).c_str() };
}

int vv_get_int(vv_object_t obj, int* readinto)
{
  if (cast_from(obj).tag() != tag::integer)
    return -1;
  *readinto = value::get<value::integer>(cast_from(obj));
  return 0;
}

int vv_get_double(vv_object_t obj, double* readinto)
{
  if (cast_from(obj).tag() != tag::floating_point)
    return -1;
  *readinto = value::get<value::floating_point>(cast_from(obj));
  return 0;
}

int vv_get_bool(vv_object_t obj, int* readinto)
{
  if (cast_from(obj).tag() != tag::boolean)
    return -1;
  *readinto = value::get<value::boolean>(cast_from(obj));
  return 0;
}

int vv_get_str(vv_object_t obj, const char** readinto)
{
  if (cast_from(obj).tag() != tag::string)
    return -1;
  *readinto = value::get<value::string>(cast_from(obj)).c_str();
  return 0;
}

int vv_get_symbol(vv_object_t obj, vv_symbol_t* readinto)
{
  if (cast_from(obj).tag() != tag::symbol)
    return -1;
  const auto cpp_sym = value::get<value::symbol>(cast_from(obj));
  readinto->string = to_string(cpp_sym).c_str();
  return 0;
}

int vv_get_blob(vv_object_t obj, void** readinto)
{
  if (cast_from(obj).tag() != tag::blob)
    return -1;
  *readinto = value::get<value::blob>(cast_from(obj)).val;
  return 0;
}

int vv_set_blob(vv_object_t obj, void* blob)
{
  if (cast_from(obj).tag() != tag::blob)
    return -1;
  value::get<value::blob>(cast_from(obj)).val = blob;
  return 0;
}

vv_object_t vv_call_fn(vv_object_t func, vv_object_t* args, size_t argc)
{
  for (auto i = argc; i--;)
    cvm().push(cast_from(args[i]));

  cvm().push(cast_from(func));
  try {
    cvm().call(static_cast<int>(argc));
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return vv_null;
  }
  return cast_to(cvm().top());
}

vv_object_t vv_get_mem(vv_object_t obj, vv_symbol_t name)
{
  if (has_member(cast_from(obj), {name.string})) {
    return cast_to(get_member(cast_from(obj), {name.string}));
  }
  return vv_null;
}

vv_object_t vv_set_mem(vv_object_t obj, vv_symbol_t name, vv_object_t val)
{
  set_member(cast_from(obj), {name.string}, cast_from(val));
  return val;
}

vv_object_t vv_get_type(vv_object_t obj)
{
  const auto type = cast_from(obj).type();
  return cast_to(type);
}

vv_object_t vv_new_object(vv_object_t type, vv_object_t* args, size_t argc)
{
  for (auto i = argc; i--;)
    cvm().push(cast_from(args[i]));

  cvm().push(cast_from(type));
  try {
    cvm().pobj(static_cast<int>(argc));
    cvm().run_cur_scope();
  } catch (const vm_error& e) {
    return vv_null;
  }
  return cast_to(cvm().top());
}

vv_object_t vv_builtin_type_array;
vv_object_t vv_builtin_type_array_iterator;
vv_object_t vv_builtin_type_bool;
vv_object_t vv_builtin_type_char;
vv_object_t vv_builtin_type_dictionary;
vv_object_t vv_builtin_type_exception;
vv_object_t vv_builtin_type_file;
vv_object_t vv_builtin_type_float;
vv_object_t vv_builtin_type_function;
vv_object_t vv_builtin_type_int;
vv_object_t vv_builtin_type_nil;
vv_object_t vv_builtin_type_range;
vv_object_t vv_builtin_type_regex;
vv_object_t vv_builtin_type_string;
vv_object_t vv_builtin_type_string_iterator;
vv_object_t vv_builtin_type_symbol;

vv_object_t vv_new_bool(int val)
{
  cvm().pbool(val);
  return cast_to(cvm().top());
}

vv_object_t vv_new_float(double val)
{
  cvm().pflt(val);
  return cast_to(cvm().top());
}

vv_object_t vv_new_file(const char* filename)
{
  const auto file = gc::alloc<value::file>( filename );
  cvm().push(file);
  return cast_to(file);
}

vv_object_t vv_new_int(int val)
{
  cvm().pint(val);
  return cast_to(cvm().top());
}

vv_object_t vv_new_nil()
{
  cvm().pnil();
  return cast_to(cvm().top());
}

vv_object_t vv_new_regex(const char* reg)
{
  try {
    cvm().pre(reg);
    return cast_to(cvm().top());
  } catch (const vm_error& e) {
    return vv_null;
  }
}

vv_object_t vv_new_string(const char* str)
{
  cvm().pstr(str);
  return cast_to(cvm().top());
}

vv_object_t vv_new_symbol(vv_symbol_t sym)
{
  cvm().psym({sym.string});
  return cast_to(cvm().top());
}

vv_object_t vv_new_blob(void* blob, void(*dtor)(void*), vv_object_t type)
{
  if (cast_from(type).tag() != tag::type)
    return vv_null;

  cvm().push(gc::alloc<value::blob>( blob, dtor ));
  cvm().top().get()->type = cast_from(type);
  return cast_to(cvm().top());
}

vv_object_t vv_alloc_blob(void* blob, void(*dtor)(void*))
{
  const auto tmp = gc::alloc<value::blob>( blob, dtor );
  return cast_to(tmp);
}

vv_object_t vv_get_parent(vv_object_t type)
{
  if (cast_from(type).tag() != tag::type)
    return vv_null;
  return cast_to(value::get<value::type>(cast_from(type)).parent);
}

vv_object_t vv_new_type(const char* name,
                        vv_object_t parent,
                        vv_object_t(*ctor)())
{
  if (cast_from(parent) && cast_from(parent).tag() != tag::type)
    return vv_null;

  const auto cpp_ctor = [ctor]() { return cast_from(ctor()); };

  const auto cpp_parent = parent ? value::get<value::type>(cast_from(parent)).parent
                                 : builtin::type::object;

  const symbol sym_name{name};
  const hash_map<symbol, gc::managed_ptr> methods{ };

  const auto type = gc::alloc<value::type>( cpp_ctor, methods, cpp_parent, sym_name );

  cvm().push(type);
  cvm().let(sym_name);
  return cast_to(type);
}

vv_object_t vv_get_arg(size_t argnum)
{
  cvm().arg(static_cast<int>(argnum));
  const auto obj = cvm().top();
  cvm().pop(1);
  return cast_to(obj);
}

vv_object_t vv_new_function(vv_object_t(func)(), size_t argc)
{
  const auto checked_fn = fn_with_err_check(func);
  const auto cpp_func = [checked_fn](vm::machine&)
  {
    return cast_from(checked_fn());
  };

  const auto fn = gc::alloc<value::builtin_function>( cpp_func, argc );
  cvm().push(fn);
  return cast_to(cvm().top());
}

vv_object_t vv_add_method(vv_object_t type,
                          const char* name,
                          vv_object_t(*func)(vv_object_t),
                          size_t argc)
{
  if (cast_from(type).tag() != tag::type)
    return vv_null;

  const auto checked_fn = fn_with_err_check(func);
  const auto cpp_func = [checked_fn](vm::machine& vm)
  {
    vm.self();
    const auto self = vm.top();
    vm.pop(1);
    return cast_from(checked_fn(cast_to(self)));
  };

  const auto fn = gc::alloc<value::builtin_function>( cpp_func, argc );
  value::get<value::type>(cast_from(type)).methods[{name}] = fn;
  return cast_to(fn);
}

vv_object_t vv_add_monop(vv_object_t type,
                         const char* name,
                         vv_object_t(*func)(vv_object_t))
{
  const auto checked = fn_with_err_check(func);
  const auto cpp_func = [checked](gc::managed_ptr self)
  {
    return cast_from(checked(cast_to(self)));
  };

  const auto fn = gc::alloc<value::opt_monop>( cpp_func );
  value::get<value::type>(cast_from(type)).methods[{name}] = fn;
  return cast_to(fn);
}

vv_object_t vv_add_binop(vv_object_t type,
                         const char* name,
                         vv_object_t(*func)(vv_object_t, vv_object_t))
{
  const auto checked = fn_with_err_check(func);
  const auto cpp_func = [checked](gc::managed_ptr lhs, gc::managed_ptr rhs)
  {
    return cast_from(checked(cast_to(lhs), cast_to(rhs)));
  };

  const auto fn = gc::alloc<value::opt_binop>( cpp_func );
  value::get<value::type>(cast_from(type)).methods[{name}] = fn;
  return cast_to(fn);
}

vv_object_t vv_let(vv_symbol_t name, vv_object_t obj)
{
  cvm().push(cast_from(obj));
  try {
    cvm().let({name.string});
  } catch (const vm_error& e) {
    return vv_null;
  }
  cvm().pop(1);
  return obj;
}

vv_object_t vv_write(vv_symbol_t name, vv_object_t obj)
{
  cvm().push(cast_from(obj));
  try {
    cvm().write({name.string});
  } catch (const vm_error& e) {
    return vv_null;
  }
  cvm().pop(1);
  return obj;
}

vv_object_t vv_read(vv_symbol_t name)
{
  try {
    cvm().read({name.string});
  } catch (const vm_error& e) {
    return vv_null;
  }
  const auto obj = cvm().top();
  cvm().pop(1);
  return cast_to(obj);
}

}
