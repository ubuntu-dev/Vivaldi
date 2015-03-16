#ifndef VV_VIVALDI_H
#define VV_VIVALDI_H

#ifdef __cplusplus

#include <cstddef>

extern "C" {

namespace vv {

namespace value {

struct base;

}

}

using vv_object_t = vv::value::base;

#else

#include <stddef.h>

struct vv_object;
typedef struct vv_object vv_object_t;

#endif

typedef struct vv_symbol {
  const char* string;
} vv_symbol_t;

vv_symbol_t vv_make_symbol(const char* string);

// Access basic types. Each returns 0 on success and -1 on failure.

int vv_get_int(vv_object_t* obj, int* readinto);
int vv_get_double(vv_object_t* obj, double* readinto);
int vv_get_bool(vv_object_t* obj, int* readinto);
int vv_get_str(vv_object_t* obj, const char** readinto);
int vv_get_symbol(vv_object_t* obj, vv_symbol_t* readinto);

// Used for custom-defined types, which store their information in a binary blob
int vv_get_blob(vv_object_t* obj, void** readinto);
int vv_set_blob(vv_object_t* obj, void* blob);

vv_object_t* vv_call_func(vv_object_t* func, vv_object_t** args, size_t argc);

vv_object_t* vv_get_mem(vv_object_t* obj, vv_symbol_t name);
vv_object_t* vv_set_mem(vv_object_t* obj, vv_symbol_t name, vv_object_t* val);
vv_object_t* vv_get_type(vv_object_t* obj);

vv_object_t* vv_new_object(vv_object_t* type, vv_object_t** args, size_t argc);

extern vv_object_t* vv_builtin_type_array;
extern vv_object_t* vv_builtin_type_array_iterator;
extern vv_object_t* vv_builtin_type_bool;
extern vv_object_t* vv_builtin_type_dictionary;
extern vv_object_t* vv_builtin_type_file;
extern vv_object_t* vv_builtin_type_float;
extern vv_object_t* vv_builtin_type_function;
extern vv_object_t* vv_builtin_type_int;
extern vv_object_t* vv_builtin_type_nil;
extern vv_object_t* vv_builtin_type_range;
extern vv_object_t* vv_builtin_type_regex;
extern vv_object_t* vv_builtin_type_string;
extern vv_object_t* vv_builtin_type_string_iterator;
extern vv_object_t* vv_builtin_type_symbol;

vv_object_t* vv_new_bool(int val);
vv_object_t* vv_new_float(double val);
vv_object_t* vv_new_file(const char* filename);
vv_object_t* vv_new_int(int quant);
vv_object_t* vv_new_nil();
vv_object_t* vv_new_regex(const char* reg);
vv_object_t* vv_new_string(const char* str);
vv_object_t* vv_new_symbol(vv_symbol_t sym);

vv_object_t* vv_new_blob(void* blob, void(*destructor)(vv_object_t*));

vv_object_t* vv_get_parent(vv_object_t* type);

vv_object_t* vv_new_type(const char* name,
                         vv_object_t* parent,
                         vv_object_t*(*constructor)(),
                         vv_object_t*(*init)(vv_object_t*),
                         size_t init_argc);

vv_object_t* vv_get_arg(size_t argnum);
vv_object_t* vv_new_function(vv_object_t*(*func)(), size_t argc);

vv_object_t* vv_add_method(vv_object_t* type,
                           const char* name,
                           vv_object_t*(*func)(vv_object_t*),
                           size_t argc);
vv_object_t* vv_add_monop(vv_object_t* type,
                          const char* name,
                          vv_object_t*(*monop)(vv_object_t*));
vv_object_t* vv_add_binop(vv_object_t* type,
                          const char* name,
                          vv_object_t*(*binop)(vv_object_t*, vv_object_t*));

vv_object_t* vv_let(vv_symbol_t name, vv_object_t* obj);
vv_object_t* vv_write(vv_symbol_t name, vv_object_t* obj);
vv_object_t* vv_read(vv_symbol_t name);

#ifdef __cplusplus

}

#endif

#endif
