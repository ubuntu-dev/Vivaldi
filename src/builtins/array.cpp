#include "builtins.h"

#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/array_iterator.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/opt_functions.h"
#include "value/type.h"

using namespace vv;
using namespace builtin;

namespace {

// Array {{{

value::basic_object* fn_array_init(vm::machine& vm)
{
  vm.self();
  auto arr = static_cast<value::array*>(vm.top());
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type != &type::array)
    return throw_exception(message::init_type_error(type::array,
                                                    type::array,
                                                    *arg->type));
  arr->val = static_cast<value::array&>(*arg).val;
  return arr;
}

value::basic_object* fn_array_size(value::basic_object* self)
{
  auto sz = static_cast<value::array&>(*self).val.size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

value::basic_object* fn_array_append(value::basic_object* self, value::basic_object* arg)
{
  static_cast<value::array&>(*self).val.push_back(arg);
  return self;
}

value::basic_object* fn_array_pop(value::basic_object* self)
{
  auto& arr = static_cast<value::array&>(*self);
  auto val = arr.val.back();
  arr.val.pop_back();
  return val;
}

value::basic_object* fn_array_at(value::basic_object* self, value::basic_object* arg)
{
  if (arg->type != &type::integer)
    return throw_exception(message::at_type_error(type::array, type::integer));
  auto val = static_cast<value::integer&>(*arg).val;
  const auto& arr = static_cast<value::array&>(*self).val;
  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception(message::out_of_range(0, arr.size(), val));
  return arr[static_cast<unsigned>(val)];
}

value::basic_object* fn_array_set_at(vm::machine& vm)
{
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type != &type::integer)
    return throw_exception(message::at_type_error(type::array, type::integer));
  auto val = static_cast<value::integer*>(arg)->val;
  vm.self();
  auto& arr = static_cast<value::array&>(*vm.top()).val;
  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception(message::out_of_range(0, arr.size(), val));
  vm.arg(1);
  return arr[static_cast<unsigned>(val)] = vm.top();
}

value::basic_object* fn_array_start(value::basic_object* self)
{
  auto arr = static_cast<value::array*>(self);
  return gc::alloc<value::array_iterator>( *arr );
}

value::basic_object* fn_array_stop(value::basic_object* self)
{
  auto arr = static_cast<value::array*>(self);
  auto iter = gc::alloc<value::array_iterator>( *arr );
  iter->idx = arr->val.size();
  return iter;
}

value::basic_object* fn_array_add(value::basic_object* self, value::basic_object* arg)
{
  auto arr = static_cast<value::array*>(self);
  if (arg->type != &type::array)
    return throw_exception(message::add_type_error(type::array, type::array));
  auto other = static_cast<value::array*>(arg);
  copy(begin(other->val), end(other->val), back_inserter(arr->val));
  return arr;
}

value::basic_object* fn_array_equals(vm::machine& vm)
{
  vm.self();
  auto self = vm.top();
  vm.arg(0);
  auto arg = vm.top();
  vm.pop(2);

  if (self == arg)
    return gc::alloc<value::boolean>( true );
  if (arg->type != &type::array)
    return gc::alloc<value::boolean>( false );

  auto& arr1 = static_cast<value::array&>(*self).val;
  auto& arr2 = static_cast<value::array&>(*arg).val;

  auto eq = std::equal(begin(arr1), end(arr1), begin(arr2), end(arr2),
                       [&](auto first, auto second)
  {
    vm.push(second);
    vm.push(first);
    vm.readm(sym::equals);
    vm.call(1);
    vm.run_cur_scope();
    auto res = vm.top();
    vm.pop(1);
    return truthy(*res);
  });

  return gc::alloc<value::boolean>( eq );
}

value::basic_object* fn_array_unequal(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !truthy(*fn_array_equals(vm)) );
}

// }}}
// Iterator {{{

value::basic_object* fn_array_iterator_at_start(value::basic_object* self)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  return gc::alloc<value::boolean>( iter.idx == 0 );
}

value::basic_object* fn_array_iterator_at_end(value::basic_object* self)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  return gc::alloc<value::boolean>( iter.idx == iter.arr.val.size() );
}

value::basic_object* fn_array_iterator_get(value::basic_object* self)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  if (iter.idx == iter.arr.val.size())
    return throw_exception(message::iterator_at_end(type::array_iterator));
  return iter.arr.val[iter.idx];
}

value::basic_object* fn_array_iterator_increment(value::basic_object* self)
{
  auto iter = static_cast<value::array_iterator*>(self);
  if (iter->idx == iter->arr.val.size())
    return throw_exception(message::iterator_past_end(type::array_iterator));
  iter->idx += 1;
  return iter;
}

value::basic_object* fn_array_iterator_decrement(value::basic_object* self)
{
  auto iter = static_cast<value::array_iterator*>(self);
  if (iter->idx == 0)
    return throw_exception(message::iterator_past_start(type::array_iterator));
  iter->idx -= 1;
  return iter;
}

value::basic_object* fn_array_iterator_add(value::basic_object* self, value::basic_object* arg)
{
  auto& iter = static_cast<value::array_iterator&>(*self);

  if (arg->type != &type::integer)
    return throw_exception(message::add_type_error(type::array, type::integer));
  auto offset = static_cast<value::integer&>(*arg).val;

  if (static_cast<int>(iter.idx) + offset < 0)
    return throw_exception(message::iterator_past_start(type::array_iterator));
  if (iter.idx + offset > iter.arr.val.size())
    return throw_exception(message::iterator_past_end(type::array_iterator));

  auto other = gc::alloc<value::array_iterator>( iter );
  static_cast<value::array_iterator&>(*other).idx = iter.idx + offset;
  return other;
}

value::basic_object* fn_array_iterator_subtract(value::basic_object* self, value::basic_object* arg)
{
  auto& iter = static_cast<value::array_iterator&>(*self);

  if (arg->type != &type::integer) {
    return throw_exception(message::add_type_error(*iter.type, type::integer));
  }

  auto offset = static_cast<value::integer&>(*arg).val;

  if (static_cast<int>(iter.idx) - offset < 0)
    return throw_exception(message::iterator_past_start(type::array_iterator));
  if (iter.idx - offset > iter.arr.val.size())
    return throw_exception(message::iterator_past_end(type::array_iterator));

  auto other = gc::alloc<value::array_iterator>( iter );
  static_cast<value::array_iterator&>(*other).idx = iter.idx - offset;
  return other;
}

value::basic_object* fn_array_iterator_equals(value::basic_object* self, value::basic_object* arg)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  auto& other = static_cast<value::array_iterator&>(*arg);
  return gc::alloc<value::boolean>(&iter.arr == &other.arr && iter.idx == other.idx);
}

value::basic_object* fn_array_iterator_unequal(value::basic_object* self, value::basic_object* arg)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  auto& other = static_cast<value::array_iterator&>(*arg);
  return gc::alloc<value::boolean>(&iter.arr != &other.arr || iter.idx != other.idx);
}

value::basic_object* fn_array_iterator_greater(value::basic_object* self, value::basic_object* arg)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  auto& other = static_cast<value::array_iterator&>(*arg);
  if (&iter.arr != &other.arr)
    return throw_exception(message::iterator_owner_error(type::array));
  return gc::alloc<value::boolean>(iter.idx > other.idx );
}

value::basic_object* fn_array_iterator_less(value::basic_object* self, value::basic_object* arg)
{
  auto& iter = static_cast<value::array_iterator&>(*self);
  auto& other = static_cast<value::array_iterator&>(*arg);
  if (&iter.arr != &other.arr)
    return throw_exception(message::iterator_owner_error(type::array));
  return gc::alloc<value::boolean>(iter.idx < other.idx );
}

// }}}

value::builtin_function array_init    {fn_array_init,    1};
value::opt_monop        array_size    {fn_array_size      };
value::opt_binop        array_append  {fn_array_append    };
value::opt_monop        array_pop     {fn_array_pop       };
value::opt_binop        array_at      {fn_array_at        };
value::builtin_function array_set_at  {fn_array_set_at,  2};
value::opt_monop        array_start   {fn_array_start     };
value::opt_monop        array_stop    {fn_array_stop      };
value::opt_binop        array_add     {fn_array_add       };
value::builtin_function array_equals  {fn_array_equals,  1};
value::builtin_function array_unequal {fn_array_unequal, 1};

value::opt_monop array_iterator_at_start  {fn_array_iterator_at_start };
value::opt_monop array_iterator_at_end    {fn_array_iterator_at_end   };
value::opt_monop array_iterator_get       {fn_array_iterator_get      };
value::opt_binop array_iterator_equals    {fn_array_iterator_equals   };
value::opt_binop array_iterator_unequal   {fn_array_iterator_unequal  };
value::opt_binop array_iterator_greater   {fn_array_iterator_greater  };
value::opt_binop array_iterator_less      {fn_array_iterator_less     };
value::opt_monop array_iterator_increment {fn_array_iterator_increment};
value::opt_monop array_iterator_decrement {fn_array_iterator_decrement};
value::opt_binop array_iterator_add       {fn_array_iterator_add      };
value::opt_binop array_iterator_subtract  {fn_array_iterator_subtract };

}

value::type type::array {gc::alloc<value::array>, {
  { {"init"},    &array_init },
  { {"size"},    &array_size },
  { {"append"},  &array_append },
  { {"pop"},     &array_pop },
  { {"at"},      &array_at },
  { {"set_at"},  &array_set_at },
  { {"start"},   &array_start },
  { {"stop"},    &array_stop },
  { {"add"},     &array_add },
  { {"equals"},  &array_equals },
  { {"unequal"}, &array_unequal },
}, type::object, {"Array"}};

value::type type::array_iterator {[]{ return nullptr; }, {
  { {"at_start"},  &array_iterator_at_start },
  { {"at_end"},    &array_iterator_at_end },
  { {"get"},       &array_iterator_get },
  { {"equals"},    &array_iterator_equals },
  { {"unequal"},   &array_iterator_unequal },
  { {"greater"},   &array_iterator_greater },
  { {"less"},      &array_iterator_less },
  { {"increment"}, &array_iterator_increment },
  { {"decrement"}, &array_iterator_decrement },
  { {"add"},       &array_iterator_add },
  { {"subtract"},  &array_iterator_subtract },
}, type::object, {"ArrayIterator"}};
