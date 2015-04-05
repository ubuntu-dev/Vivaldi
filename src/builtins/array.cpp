#include "builtins/array.h"

#include "builtins.h"
#include "messages.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/array_iterator.h"

using namespace vv;
using namespace builtin;

// Array

gc::managed_ptr array::init(vm::machine& vm)
{
  vm.self();
  const auto self = vm.top();
  vm.arg(0);
  const auto arg = vm.top();
  if (arg.tag() != tag::array)
    return throw_exception(message::init_type_error(type::array,
                                                    type::array,
                                                    arg.type()));
  value::get<value::array>(self) = value::get<value::array>(arg);
  return self;
}

gc::managed_ptr array::size(gc::managed_ptr self)
{
  const auto sz = value::get<value::array>(self).size();
  return gc::alloc<value::integer>( static_cast<int>(sz) );
}

gc::managed_ptr array::append(gc::managed_ptr self, gc::managed_ptr arg)
{
  value::get<value::array>(self).push_back(arg);
  return self;
}

gc::managed_ptr array::pop(gc::managed_ptr self)
{
  auto& arr = value::get<value::array>(self);
  const auto val = arr.back();
  arr.pop_back();
  return val;
}

gc::managed_ptr array::at(gc::managed_ptr self, gc::managed_ptr arg)
{
  if (arg.tag() != tag::integer)
    return throw_exception(message::at_type_error(type::array, type::integer));

  const auto val = value::get<value::integer>(arg);
  const auto& arr = value::get<value::array>(self);

  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception(message::out_of_range(0, arr.size(), val));

  return arr[static_cast<unsigned>(val)];
}

gc::managed_ptr array::set_at(vm::machine& vm)
{
  vm.arg(0);
  auto arg = vm.top();

  if (arg.tag() != tag::integer)
    return throw_exception(message::at_type_error(type::array, type::integer));

  const auto val = value::get<value::integer>(arg);

  vm.self();
  auto& arr = value::get<value::array>(vm.top());

  if (arr.size() <= static_cast<unsigned>(val) || val < 0)
    return throw_exception(message::out_of_range(0, arr.size(), val));

  vm.arg(1);
  return arr[static_cast<unsigned>(val)] = vm.top();
}

gc::managed_ptr array::start(gc::managed_ptr self)
{
  return gc::alloc<value::array_iterator>( self );
}

gc::managed_ptr array::stop(gc::managed_ptr self)
{
  const auto& arr = value::get<value::array>(self);
  auto iter = gc::alloc<value::array_iterator>( self );
  value::get<value::array_iterator>(iter).idx = arr.size();
  return iter;
}

gc::managed_ptr array::add(gc::managed_ptr self, gc::managed_ptr arg)
{
  auto arr = value::get<value::array>(self);

  if (arg.tag() != tag::array)
    return throw_exception(message::add_type_error(type::array, type::array));
  const auto& other = value::get<value::array>(arg);

  copy(begin(other), end(other), back_inserter(arr));
  return gc::alloc<value::array>( arr );
}

gc::managed_ptr array::equals(vm::machine& vm)
{
  vm.self();
  auto self = vm.top();
  vm.arg(0);
  auto arg = vm.top();
  vm.pop(2);

  if (self == arg)
    return gc::alloc<value::boolean>( true );
  if (arg.tag() != tag::array)
    return gc::alloc<value::boolean>( false );

  const auto& arr1 = value::get<value::array>(self);
  const auto& arr2 = value::get<value::array>(arg);

  auto eq = std::equal(begin(arr1), end(arr1), begin(arr2), end(arr2),
                       [&](auto first, auto second)
  {
    vm.push(second);
    vm.push(first);
    vm.method(sym::equals);
    vm.call(1);
    vm.run_cur_scope();
    auto res = vm.top();
    vm.pop(1);
    return truthy(res);
  });

  return gc::alloc<value::boolean>( eq );
}

gc::managed_ptr array::unequal(vm::machine& vm)
{
  return gc::alloc<value::boolean>( !truthy(array::equals(vm)) );
}

// Iterator

gc::managed_ptr array_iterator::at_start(gc::managed_ptr self)
{
  const auto& iter = value::get<value::array_iterator>(self);
  return gc::alloc<value::boolean>( iter.idx == 0 );
}

gc::managed_ptr array_iterator::at_end(gc::managed_ptr self)
{
  const auto& iter = value::get<value::array_iterator>(self);
  return gc::alloc<value::boolean>( iter.idx ==
                                    value::get<value::array>(iter.arr).size() );
}

gc::managed_ptr array_iterator::get(gc::managed_ptr self)
{
  const auto& iter = value::get<value::array_iterator>(self);
  if (iter.idx == value::get<value::array>(iter.arr).size())
    return throw_exception(message::iterator_at_end(type::array_iterator));
  return value::get<value::array>(iter.arr)[iter.idx];
}

gc::managed_ptr array_iterator::increment(gc::managed_ptr self)
{
  auto& iter = value::get<value::array_iterator>(self);
  if (iter.idx == value::get<value::array>(iter.arr).size())
    return throw_exception(message::iterator_past_end(type::array_iterator));

  ++iter.idx;
  return self;
}

gc::managed_ptr array_iterator::decrement(gc::managed_ptr self)
{
  auto& iter = value::get<value::array_iterator>(self);
  if (iter.idx == 0)
    return throw_exception(message::iterator_past_start(type::array_iterator));
  --iter.idx;
  return self;
}

gc::managed_ptr array_iterator::add(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::array_iterator>(self);

  if (arg.tag() != tag::integer)
    return throw_exception(message::add_type_error(type::array, type::integer));
  auto offset = value::get<value::integer>(arg);

  if (static_cast<int>(iter.idx) + offset < 0)
    return throw_exception(message::iterator_past_start(type::array_iterator));
  if (iter.idx + offset > value::get<value::array>(iter.arr).size())
    return throw_exception(message::iterator_past_end(type::array_iterator));

  auto other = gc::alloc<value::array_iterator>( iter.arr );
  value::get<value::array_iterator>(other).idx = iter.idx + offset;
  return other;
}

gc::managed_ptr array_iterator::subtract(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::array_iterator>(self);

  if (arg.tag() != tag::integer) {
    return throw_exception(message::add_type_error(self.type(), type::integer));
  }

  const auto offset = value::get<value::integer>(arg);

  if (static_cast<int>(iter.idx) - offset < 0)
    return throw_exception(message::iterator_past_start(type::array_iterator));
  if (iter.idx - offset > value::get<value::array>(iter.arr).size())
    return throw_exception(message::iterator_past_end(type::array_iterator));

  const auto other = gc::alloc<value::array_iterator>( iter.arr );
  value::get<value::array_iterator>(other).idx = iter.idx - offset;
  return other;
}

gc::managed_ptr array_iterator::equals(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::array_iterator>(self);
  const auto& other = value::get<value::array_iterator>(arg);
  return gc::alloc<value::boolean>(iter.arr == other.arr && iter.idx == other.idx);
}

gc::managed_ptr array_iterator::unequal(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::array_iterator>(self);
  const auto& other = value::get<value::array_iterator>(arg);
  return gc::alloc<value::boolean>(iter.arr != other.arr || iter.idx != other.idx);
}

gc::managed_ptr array_iterator::greater(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::array_iterator>(self);
  const auto& other = value::get<value::array_iterator>(arg);
  if (iter.arr != other.arr)
    return throw_exception(message::iterator_owner_error(type::array));
  return gc::alloc<value::boolean>(iter.idx > other.idx );
}

gc::managed_ptr array_iterator::less(gc::managed_ptr self, gc::managed_ptr arg)
{
  const auto& iter = value::get<value::array_iterator>(self);
  const auto& other = value::get<value::array_iterator>(arg);
  if (iter.arr != other.arr)
    return throw_exception(message::iterator_owner_error(type::array));
  return gc::alloc<value::boolean>(iter.idx < other.idx );
}
