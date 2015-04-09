#include "builtins.h"

#include "c_internal.h"
#include "builtins/array.h"
#include "builtins/dictionary.h"
#include "builtins/file.h"
#include "builtins/floating_point.h"
#include "builtins/integer.h"
#include "builtins/object.h"
#include "builtins/range.h"
#include "builtins/regex.h"
#include "builtins/string.h"
#include "builtins/symbol.h"
#include "builtins/type.h"
#include "gc/alloc.h"
#include "utils/lang.h"
#include "value/array.h"
#include "value/builtin_function.h"
#include "value/dictionary.h"
#include "value/file.h"
#include "value/function.h"
#include "value/object.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/symbol.h"
#include "value/type.h"

#include <iostream>

using namespace vv;
using namespace builtin;

// Symbols {{{

const vv::symbol sym::self{"self"};
const vv::symbol sym::call{"call"};

const vv::symbol sym::start{"start"};
const vv::symbol sym::at_end{"at_end"};
const vv::symbol sym::get{"get"};
const vv::symbol sym::increment{"increment"};

const vv::symbol sym::size{"size"};

const vv::symbol sym::add{"add"};
const vv::symbol sym::subtract{"subtract"};
const vv::symbol sym::times{"times"};
const vv::symbol sym::divides{"divides"};

const vv::symbol sym::equals{"equals"};
const vv::symbol sym::unequal{"unequal"};

const vv::symbol sym::op_not{"not"};

const vv::symbol sym::greater{"greater"};
const vv::symbol sym::less{"less"};
const vv::symbol sym::greater_equals{"greater_equals"};
const vv::symbol sym::less_equals{"less_equals"};
// }}}
// Freestanding functions {{{

namespace {

// Helper functions {{{

struct call_result {
  bool completed;
  gc::managed_ptr value;
};

call_result call_method(vm::machine& vm,
                        gc::managed_ptr self,
                        const vv::symbol method)
{
  vm.push(self);
  vm.method(method);
  vm.call(0);
  vm.run_cur_scope();
  const auto val = vm.top();
  vm.pop(1);
  return { true, val };
}

template <typename F>
call_result fake_for_loop(vm::machine& vm, const F& inner)
{
  // Get iterator from range
  vm.arg(0);
  const auto range = vm.top();
  auto iter = call_method(vm, range, sym::start).value;

  vm.arg(1);
  const auto supplied_fn = vm.top();

  for (;;) {
    vm.push(iter);
    vm.opt_at_end();
    const auto at_end = vm.top();
    vm.pop(1);
    if (truthy(at_end)) {
      return { true, at_end };
    }

    vm.push(iter);
    vm.opt_get();
    const auto next_item = vm.top();
    vm.pop(1);

    const auto res = inner(vm, supplied_fn, next_item);
    if (res.completed) {
      vm.pop(1); // iter
      return res;
    }

    vm.push(iter);
    vm.opt_incr();
    vm.pop(1);
  }
}

template <typename F>
call_result transformed_range(vm::machine& vm, const F& inner)
{
  return fake_for_loop(vm, [&](auto& vm, auto transform, auto orig)
  {
    vm.push(orig);

    vm.push(transform);
    vm.call(1);
    vm.run_cur_scope();

    const auto completed = inner(orig, vm.top());
    return call_result{ completed, vm.top() };
  });
}

// }}}
// I/O {{{

gc::managed_ptr fn_print(vm::machine& vm)
{
  vm.arg(0);
  const auto arg = vm.top();

  if (arg.tag() == tag::string)
    std::cout << value::get<value::string>(arg);
  else
    std::cout << pretty_print(arg, vm);
  return gc::alloc<value::nil>( );
}

gc::managed_ptr fn_puts(vm::machine& vm)
{
  const auto ret = fn_print(vm);
  std::cout << '\n';
  return ret;
}

gc::managed_ptr fn_gets(vm::machine&)
{
  std::string str;
  getline(std::cin, str);

  return gc::alloc<value::string>( str );
}

// }}}
// Functional stuff {{{

gc::managed_ptr fn_filter(vm::machine& vm)
{
  vm.parr(0);
  const auto array = vm.top();

  transformed_range(vm, [array](auto item, auto pred)
  {
    if (truthy(pred))
      value::get<value::array>(array).push_back(item);
    return false;
  });

  return array;
}

gc::managed_ptr fn_map(vm::machine& vm)
{
  // Get pointer to empty Array
  vm.parr(0);
  const auto mapped = vm.top();

  transformed_range(vm, [mapped](auto, auto val)
  {
    value::get<value::array>(mapped).push_back(val);
    return false;
  });

  vm.pop(1); // filtered
  return mapped;
}

gc::managed_ptr fn_count(vm::machine& vm)
{
  int count{};
  transformed_range(vm, [&](auto, auto pred)
                           { if (truthy(pred)) ++count; return false; });
  return gc::alloc<value::integer>( count );
}

gc::managed_ptr fn_all(vm::machine& vm)
{
  auto all = true;
  transformed_range(vm, [&](auto, auto pred)
                           { if (!truthy(pred)) all = false; return !all; });
  return gc::alloc<value::boolean>( all );
}

gc::managed_ptr fn_any(vm::machine& vm)
{
  auto found = false;
  transformed_range(vm, [&](auto, auto pred)
                           { if (truthy(pred)) found = true; return found; });
  return gc::alloc<value::boolean>( found );
}

gc::managed_ptr fn_reduce(vm::machine& vm)
{
  // Get iterator from range
  vm.arg(0);
  const auto range = vm.top();
  vm.pop(1);
  const auto iter = call_method(vm, range, sym::start).value;
  vm.push(iter);

  vm.arg(1); // put total on stack


  for (;;) {
    vm.push(iter);
    vm.opt_at_end();
    const auto at_end = vm.top();
    vm.pop(1);
    if (truthy(at_end)) {
      return vm.top();
    }

    auto total = vm.top();
    vm.push(iter);
    vm.opt_get();
    const auto next_item = vm.top();
    vm.pop(1);
    vm.push(next_item);
    vm.push(total);

    vm.arg(2); // supplied function
    vm.call(2);
    vm.run_cur_scope();

    // replace old total with new
    total = vm.top();
    vm.pop(2); // new total, orig total
    vm.push(total);

    vm.push(iter);
    vm.opt_incr();
    vm.pop(1);
  }
}

gc::managed_ptr fn_sort(vm::machine& vm)
{
  vm.parr(0);
  const auto array = vm.top();

  vm.arg(0);
  const auto range = vm.top();
  vm.pop(1);
  const auto iter = call_method(vm, range, sym::start).value;
  vm.push(iter);

  for (;;) {
    vm.push(iter);
    vm.opt_at_end();
    const auto at_end = vm.top();
    vm.pop(1);
    if (truthy(at_end))
      break;

    vm.push(iter);
    vm.opt_get();
    const auto next_item = vm.top();
    vm.pop(1);
    value::get<value::array>(array).push_back(next_item);

    vm.push(iter);
    vm.opt_incr();
    vm.pop(1);
  }

  std::sort(begin(value::get<value::array>(array)),
            end(value::get<value::array>(array)),
            [&](auto left, auto right)
  {
    vm.push(right);
    vm.push(left);
    vm.method(sym::less);
    vm.call(1);
    vm.run_cur_scope();
    const auto res = vm.top();
    vm.pop(1);
    return truthy(res);
  });

  return array;
}

// }}}
// Other {{{

gc::managed_ptr fn_quit(vm::machine&)
{
  exit(0);
}

gc::managed_ptr fn_reverse(vm::machine& vm)
{
  // Get iterator from range
  vm.arg(0);
  const auto range = vm.top();
  const auto iter = call_method(vm, range, sym::start).value;

  vm.parr(0);
  const auto arr = vm.top();

  for (;;) {
    vm.push(iter);
    vm.opt_at_end();
    const auto at_end = vm.top();
    vm.pop(1);
    if (truthy(at_end)) {
      vm.pop(1); // iter
      reverse(begin(value::get<value::array>(arr)), end(value::get<value::array>(arr)));
      return arr;
    }

    vm.push(iter);
    vm.opt_get();
    const auto next_item = vm.top();
    vm.pop(1);
    value::get<value::array>(arr).push_back(next_item);

    vm.push(iter);
    vm.opt_incr();
    vm.pop(1);
  }
}

// }}}

}

gc::managed_ptr function::print;
gc::managed_ptr function::puts;
gc::managed_ptr function::gets;

gc::managed_ptr function::filter;
gc::managed_ptr function::map;
gc::managed_ptr function::reduce;
gc::managed_ptr function::sort;
gc::managed_ptr function::all;
gc::managed_ptr function::any;
gc::managed_ptr function::count;

gc::managed_ptr function::quit;
gc::managed_ptr function::reverse;

// }}}
// Types {{{

gc::managed_ptr type::array;
gc::managed_ptr type::array_iterator;
gc::managed_ptr type::boolean;
gc::managed_ptr type::dictionary;
gc::managed_ptr type::custom_type;
gc::managed_ptr type::file;
gc::managed_ptr type::floating_point;
gc::managed_ptr type::function;
gc::managed_ptr type::integer;
gc::managed_ptr type::object;
gc::managed_ptr type::nil;
gc::managed_ptr type::range;
gc::managed_ptr type::regex;
gc::managed_ptr type::regex_result;
gc::managed_ptr type::string;
gc::managed_ptr type::string_iterator;
gc::managed_ptr type::symbol;

// }}}
// init {{{

// Individual class initialization functions {{{

namespace {

void init_array()
{
  const auto init = gc::alloc<value::opt_binop>( array::init );
  const auto size = gc::alloc<value::opt_monop>( array::size );
  const auto append = gc::alloc<value::opt_binop>( array::append );
  const auto pop = gc::alloc<value::opt_monop>( array::pop );
  const auto at = gc::alloc<value::opt_binop>( array::at );
  const auto set_at = gc::alloc<value::builtin_function>( array::set_at, size_t{2} );
  const auto start = gc::alloc<value::opt_monop>( array::start );
  const auto stop = gc::alloc<value::opt_monop>( array::stop );
  const auto add = gc::alloc<value::opt_binop>( array::add );
  const auto equals = gc::alloc<value::builtin_function>( array::equals, size_t{1} );
  const auto unequal = gc::alloc<value::builtin_function>( array::unequal, size_t{1} );

  builtin::type::array = gc::alloc<value::type>(
      gc::alloc<value::array>,
      hash_map<vv::symbol, gc::managed_ptr>{
        { {"init"}, init },
        { {"size"}, size },
        { {"append"}, append },
        { {"pop"}, pop },
        { {"at"}, at },
        { {"set_at"}, set_at },
        { {"start"}, start },
        { {"stop"}, stop },
        { {"add"}, add },
        { {"equals"}, equals },
        { {"unequal"}, unequal }
      },
      type::object,
      vv::symbol{"Array"});
}

void init_array_iterator()
{
  const auto at_start = gc::alloc<value::opt_monop>( array_iterator::at_start );
  const auto at_end = gc::alloc<value::opt_monop>( array_iterator::at_end );
  const auto get = gc::alloc<value::opt_monop>( array_iterator::get );
  const auto equals = gc::alloc<value::opt_binop>( array_iterator::equals );
  const auto unequal = gc::alloc<value::opt_binop>( array_iterator::unequal );
  const auto greater = gc::alloc<value::opt_binop>( array_iterator::greater );
  const auto less = gc::alloc<value::opt_binop>( array_iterator::less );
  const auto increment = gc::alloc<value::opt_monop>( array_iterator::increment );
  const auto decrement = gc::alloc<value::opt_monop>( array_iterator::decrement );
  const auto add = gc::alloc<value::opt_binop>( array_iterator::add );
  const auto subtract = gc::alloc<value::opt_binop>( array_iterator::subtract );

  builtin::type::array_iterator = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr>{
        { {"at_start"}, at_start },
        { {"at_end"}, at_end },
        { {"get"}, get },
        { {"equals"}, equals },
        { {"unequal"}, unequal },
        { {"greater"}, greater },
        { {"less"}, less },
        { {"increment"}, increment },
        { {"decrement"}, decrement },
        { {"add"}, add },
        { {"subtract"}, subtract }
      },
      type::object,
      vv::symbol{"ArrayIterator"});
}

void init_boolean()
{
  builtin::type::boolean = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr>{ },
      type::object,
      vv::symbol{"Bool"});
}

void init_dictionary()
{
  const auto init = gc::alloc<value::opt_binop>( dictionary::init );
  const auto size = gc::alloc<value::opt_monop>( dictionary::size );
  const auto at = gc::alloc<value::opt_binop>( dictionary::at );
  const auto set_at = gc::alloc<value::builtin_function>( dictionary::set_at, size_t{2} );

  builtin::type::dictionary = gc::alloc<value::type>(
      gc::alloc<value::dictionary>,
      hash_map<vv::symbol, gc::managed_ptr>{
        { {"init"}, init },
        { {"size"}, size },
        { {"at"}, at },
        { {"set_at"}, set_at }
      },
      type::object,
      vv::symbol{"Dictionary"});
}

void init_file()
{
  const auto init = gc::alloc<value::opt_binop>( file::init );
  const auto contents = gc::alloc<value::opt_monop>( file::contents );
  const auto start = gc::alloc<value::opt_monop>( file::start );
  const auto get = gc::alloc<value::opt_monop>( file::get );
  const auto increment = gc::alloc<value::opt_monop>( file::increment );
  const auto at_end = gc::alloc<value::opt_monop>( file::at_end );

  builtin::type::file = gc::alloc<value::type>(
      gc::alloc<value::file>,
      hash_map<vv::symbol, gc::managed_ptr>{
        { {"init"}, init },
        { {"contents"}, contents },
        { {"start"}, start },
        { {"get"}, get },
        { {"increment"}, increment },
        { {"at_end"}, at_end }
      },
      type::object,
      vv::symbol{"File"});
}

void init_floating_point()
{
  const auto add = gc::alloc<value::opt_binop>( floating_point::add );
  const auto subtract = gc::alloc<value::opt_binop>( floating_point::subtract );
  const auto times = gc::alloc<value::opt_binop>( floating_point::times );
  const auto divides = gc::alloc<value::opt_binop>( floating_point::divides );
  const auto pow = gc::alloc<value::opt_binop>( floating_point::pow );

  const auto equals = gc::alloc<value::opt_binop>( floating_point::equals );
  const auto unequal = gc::alloc<value::opt_binop>( floating_point::unequal );
  const auto less = gc::alloc<value::opt_binop>( floating_point::less );
  const auto greater = gc::alloc<value::opt_binop>( floating_point::greater );
  const auto less_equals = gc::alloc<value::opt_binop>( floating_point::less_equals );
  const auto greater_equals = gc::alloc<value::opt_binop>( floating_point::greater_equals );

  const auto negative = gc::alloc<value::opt_monop>( floating_point::negative );
  const auto sqrt = gc::alloc<value::opt_monop>( floating_point::sqrt );
  const auto sin = gc::alloc<value::opt_monop>( floating_point::sin );
  const auto cos = gc::alloc<value::opt_monop>( floating_point::cos );
  const auto tan = gc::alloc<value::opt_monop>( floating_point::tan );

  builtin::type::floating_point = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr>{
        { {"add"}, add },
        { {"subtract"}, subtract },
        { {"times"}, times },
        { {"divides"}, divides },
        { {"pow"}, pow },

        { {"equals"}, equals },
        { {"unequal"}, unequal },
        { {"less"}, less },
        { {"greater"}, greater },
        { {"less_equals"}, less_equals },
        { {"greater_equals"}, greater_equals },

        { {"negative"}, negative },
        { {"sqrt"}, sqrt },
        { {"sin"}, sin },
        { {"cos"}, cos },
        { {"tan"}, tan }
      },
      type::object,
      vv::symbol{"Float"});
}

void init_function()
{
  builtin::type::function = gc::alloc<value::type>(
      []{ return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr>{},
      type::object,
      vv::symbol{"Function"});
}

void init_integer()
{
  const auto add = gc::alloc<value::opt_binop>( integer::add );
  const auto subtract = gc::alloc<value::opt_binop>( integer::subtract );
  const auto times = gc::alloc<value::opt_binop>( integer::times );
  const auto divides = gc::alloc<value::opt_binop>( integer::divides );
  const auto modulo = gc::alloc<value::opt_binop>( integer::modulo );
  const auto pow = gc::alloc<value::opt_binop>( integer::pow );

  const auto lshift = gc::alloc<value::opt_binop>( integer::lshift );
  const auto rshift = gc::alloc<value::opt_binop>( integer::rshift );
  const auto bit_and = gc::alloc<value::opt_binop>( integer::bit_and );
  const auto bit_or = gc::alloc<value::opt_binop>( integer::bit_or );
  const auto x_or = gc::alloc<value::opt_binop>( integer::x_or );

  const auto equals = gc::alloc<value::opt_binop>( integer::equals );
  const auto unequal = gc::alloc<value::opt_binop>( integer::unequal );
  const auto less = gc::alloc<value::opt_binop>( integer::less );
  const auto greater = gc::alloc<value::opt_binop>( integer::greater );
  const auto less_equals = gc::alloc<value::opt_binop>( integer::less_equals );
  const auto greater_equals = gc::alloc<value::opt_binop>( integer::greater_equals );

  const auto negative = gc::alloc<value::opt_monop>( integer::negative );
  const auto negate = gc::alloc<value::opt_monop>( integer::negate );
  const auto sqrt = gc::alloc<value::opt_monop>( integer::sqrt );
  const auto sin = gc::alloc<value::opt_monop>( integer::sin );
  const auto cos = gc::alloc<value::opt_monop>( integer::cos );
  const auto tan = gc::alloc<value::opt_monop>( integer::tan );
  const auto chr = gc::alloc<value::opt_monop>( integer::chr );

  builtin::type::integer = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"add"}, add },
        { {"subtract"}, subtract },
        { {"times"}, times },
        { {"divides"}, divides },
        { {"modulo"}, modulo },
        { {"pow"}, pow },

        { {"lshift"}, lshift },
        { {"rshift"}, rshift },
        { {"bitand"}, bit_and },
        { {"bitor"}, bit_or },
        { {"xor"}, x_or },

        { {"equals"}, equals },
        { {"unequal"}, unequal },
        { {"less"}, less },
        { {"greater"}, greater },
        { {"less_equals"}, less_equals },
        { {"greater_equals"}, greater_equals },

        { {"negative"}, negative },
        { {"negate"}, negate },
        { {"sqrt"}, sqrt },
        { {"sin"}, sin },
        { {"cos"}, cos },
        { {"tan"}, tan },
        { {"chr"}, chr }
      },
      builtin::type::object,
      vv::symbol{"Integer"} );
}

void init_nil()
{
  builtin::type::nil = gc::alloc<value::type>(
      []{ return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr>{},
      type::object,
      vv::symbol{"Nil"});
}

void init_object()
{
  const auto equals = gc::alloc<value::opt_binop>( object::equals );
  const auto unequal = gc::alloc<value::opt_binop>( object::unequal );
  const auto op_not = gc::alloc<value::opt_monop>( object::op_not );
  const auto type = gc::alloc<value::opt_monop>( object::type );
  const auto member = gc::alloc<value::opt_binop>( object::member );
  const auto has_member = gc::alloc<value::opt_binop>( object::has_member );
  const auto set_member = gc::alloc<value::builtin_function>( object::set_member, size_t{2} );

  builtin::type::object = gc::alloc<value::type>(
      gc::alloc<value::object>,
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"equals"}, equals },
        { {"unequal"}, unequal },
        { {"not"}, op_not },
        { {"type"}, type },
        { {"member"}, member },
        { {"has_member"}, has_member },
        { {"set_member"}, set_member }
      },
      builtin::type::object,
      vv::symbol{"Object"} );
}

void init_range()
{
  const auto init = gc::alloc<value::builtin_function>( range::init, size_t{2} );
  const auto start = gc::alloc<value::opt_monop>( range::start );
  const auto size = gc::alloc<value::builtin_function>( range::size, size_t{0} );
  const auto at_end = gc::alloc<value::builtin_function>( range::at_end, size_t{0} );
  const auto get = gc::alloc<value::opt_monop>( range::get );
  const auto increment = gc::alloc<value::builtin_function>( range::increment, size_t{0} );
  const auto to_arr = gc::alloc<value::builtin_function>( range::to_arr, size_t{0} );

  builtin::type::range = gc::alloc<value::type>(
      gc::alloc<value::range>,
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"init"}, init },
        { {"start"}, start },
        { {"size"}, size },
        { {"at_end"}, at_end },
        { {"get"}, get },
        { {"increment"}, increment },
        { {"to_arr"}, to_arr }
      },
      builtin::type::object,
      vv::symbol{"Range"} );
}

void init_regex()
{
  const auto init = gc::alloc<value::builtin_function>( regex::init, size_t{1} );
  const auto match = gc::alloc<value::opt_binop>( regex::match );
  const auto match_index = gc::alloc<value::opt_binop>( regex::match_index );

  builtin::type::regex = gc::alloc<value::type>(
      gc::alloc<value::regex>,
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"init"}, init },
        { {"match"}, match },
        { {"match_index"}, match_index },
      },
      builtin::type::object,
      vv::symbol{"RegEx"} );
}

void init_regex_result()
{
  const auto at = gc::alloc<value::opt_binop>( regex_result::at );
  const auto index = gc::alloc<value::opt_binop>( regex_result::index );
  const auto size = gc::alloc<value::opt_monop>( regex_result::size );

  builtin::type::regex_result = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"at"}, at },
        { {"index"}, index },
        { {"size"}, size },
      },
      builtin::type::object,
      vv::symbol{"RegExResult"} );
}

void init_string()
{
  const auto init = gc::alloc<value::opt_binop>( string::init );
  const auto size = gc::alloc<value::opt_monop>( string::size );

  const auto equals = gc::alloc<value::opt_binop>( string::equals );
  const auto unequal = gc::alloc<value::opt_binop>( string::unequal );
  const auto less = gc::alloc<value::opt_binop>( string::less );
  const auto greater = gc::alloc<value::opt_binop>( string::greater );
  const auto less_equals = gc::alloc<value::opt_binop>( string::less_equals );
  const auto greater_equals = gc::alloc<value::opt_binop>( string::greater_equals );

  const auto add = gc::alloc<value::opt_binop>( string::add );
  const auto times = gc::alloc<value::opt_binop>( string::times );

  const auto to_int = gc::alloc<value::opt_monop>( string::to_int );
  const auto to_flt = gc::alloc<value::opt_monop>( string::to_flt );

  const auto at = gc::alloc<value::opt_binop>( string::at );
  const auto start = gc::alloc<value::opt_monop>( string::start );
  const auto stop = gc::alloc<value::opt_monop>( string::stop );

  const auto to_upper = gc::alloc<value::opt_monop>( string::to_upper );
  const auto to_lower = gc::alloc<value::opt_monop>( string::to_lower );

  const auto starts_with = gc::alloc<value::opt_binop>( string::starts_with );

  const auto ord = gc::alloc<value::opt_monop>( string::ord );

  const auto split = gc::alloc<value::builtin_function>( string::split, size_t{1} );
  const auto replace = gc::alloc<value::builtin_function>( string::replace, size_t{2} );

  builtin::type::string = gc::alloc<value::type>(
      gc::alloc<value::string>,
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"init"}, init },
        { {"size"}, size },

        { {"equals"}, equals },
        { {"unequal"}, unequal },
        { {"less"}, less },
        { {"greater"}, greater },
        { {"less_equals"}, less_equals },
        { {"greater_equals"}, greater_equals },

        { {"add"}, add },
        { {"times"}, times },

        { {"to_int"}, to_int },
        { {"to_flt"}, to_flt },

        { {"at"}, at },
        { {"start"}, start },
        { {"stop"}, stop },

        { {"to_upper"}, to_upper },
        { {"to_lower"}, to_lower },

        { {"starts_with"}, starts_with },

        { {"ord"}, ord },

        { {"split"}, split },
        { {"replace"}, replace }
      },
      builtin::type::object,
      vv::symbol{"String"} );
}

void init_string_iterator()
{
  const auto at_start = gc::alloc<value::opt_monop>( string_iterator::at_start );
  const auto at_end = gc::alloc<value::opt_monop>( string_iterator::at_end );
  const auto get = gc::alloc<value::opt_monop>( string_iterator::get );

  const auto equals = gc::alloc<value::opt_binop>( string_iterator::equals );
  const auto unequal = gc::alloc<value::opt_binop>( string_iterator::unequal );

  const auto increment = gc::alloc<value::opt_monop>( string_iterator::increment );
  const auto decrement = gc::alloc<value::opt_monop>( string_iterator::decrement );

  const auto add = gc::alloc<value::opt_binop>( string_iterator::add );
  const auto subtract = gc::alloc<value::opt_binop>( string_iterator::subtract );

  builtin::type::string_iterator = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"at_start"}, at_start },
        { {"at_end"}, at_end },
        { {"get"}, get },

        { {"equals"}, equals },
        { {"unequal"}, unequal },

        { {"increment"}, increment },
        { {"decrement"}, decrement },

        { {"add"}, add },
        { {"subtract"}, subtract }
      },
      builtin::type::object,
      vv::symbol{"StringIterator"} );
}

void init_symbol()
{
  const auto init = gc::alloc<value::opt_binop>( builtin::symbol::init );
  const auto equals = gc::alloc<value::opt_binop>( builtin::symbol::equals );
  const auto unequal = gc::alloc<value::opt_binop>( builtin::symbol::unequal );

  builtin::type::symbol = gc::alloc<value::type>(
      gc::alloc<value::symbol>,
      hash_map<vv::symbol, gc::managed_ptr> {
        { {"init"}, init },
        { {"equals"}, equals },
        { {"unequal"}, unequal }
      },
      builtin::type::object,
      vv::symbol{"Symbol"} );
}

void init_type()
{
  // 'custom_' because the 'type' namespace is used for builtin types
  auto parent = gc::alloc<value::opt_monop>( custom_type::parent );
  builtin::type::custom_type = gc::alloc<value::type>(
      [] { return gc::managed_ptr{}; },
      hash_map<vv::symbol, gc::managed_ptr> { { {"parent"}, parent }, },
      builtin::type::object,
      vv::symbol{"Type"} );
}

}

// }}}

void builtin::init()
{
  // XXX: This gets slightly hairy, since many constructors depend on the values
  // this function initializes already being initialized. Whoops.
  //
  // Things to keep in mind:
  // - a type's init method *must* be passed in its constructor
  // - if an object is instantiated before its VV type, its type has to be
  //   corrected post facto
  // - ditto a type's parent

  // Most of what we'll be creating is functions, so get that type out of the
  // way first. Remember we'll have to correct its parent once type::object is
  // allocated, and its type once type::custom_type is allocated!
  init_function();

  // Start with type::object, since it's the next most important
  init_object();
  // Fix parent types, now that Object is allocated
  value::get<value::type>(builtin::type::object).parent = builtin::type::object;
  value::get<value::type>(builtin::type::function).parent = builtin::type::object;

  // Object and Function's types are still screwed up, so deal with that next
  init_type();
  // Fix the types of already-allocated Types
  builtin::type::function.get()->type = builtin::type::custom_type;
  builtin::type::object.get()->type = builtin::type::custom_type;
  builtin::type::custom_type.get()->type = builtin::type::custom_type;

  // Now all the weird timing stuff should be out of the way; the rest is
  // relatively straightforward
  init_array();
  init_array_iterator();
  init_boolean();
  init_dictionary();
  init_file();
  init_floating_point();
  init_integer();
  init_nil();
  init_range();
  init_regex();
  init_regex_result();
  init_string();
  init_string_iterator();
  init_symbol();

  // Now allocated the standalone functions
  function::print = gc::alloc<value::builtin_function>( fn_print, size_t{1} );
  function::puts = gc::alloc<value::builtin_function>( fn_puts, size_t{1} );
  function::gets = gc::alloc<value::builtin_function>( fn_gets, size_t{0} );

  function::filter = gc::alloc<value::builtin_function>( fn_filter, size_t{2} );
  function::map = gc::alloc<value::builtin_function>( fn_map, size_t{2} );
  function::reduce = gc::alloc<value::builtin_function>( fn_reduce, size_t{3} );
  function::sort = gc::alloc<value::builtin_function>( fn_sort, size_t{1} );
  function::all = gc::alloc<value::builtin_function>( fn_all, size_t{2} );
  function::any = gc::alloc<value::builtin_function>( fn_any, size_t{2} );
  function::count = gc::alloc<value::builtin_function>( fn_count, size_t{2} );

  function::quit = gc::alloc<value::builtin_function>( fn_quit, size_t{0} );
  function::reverse = gc::alloc<value::builtin_function>( fn_reverse, size_t{1} );

  // Finally, initialize the C API pointers to builtin types

  vv_builtin_type_array           = cast_to(type::array);
  vv_builtin_type_array_iterator  = cast_to(type::array_iterator);
  vv_builtin_type_bool            = cast_to(type::boolean);
  vv_builtin_type_dictionary      = cast_to(type::dictionary);
  vv_builtin_type_file            = cast_to(type::file);
  vv_builtin_type_float           = cast_to(type::floating_point);
  vv_builtin_type_function        = cast_to(type::function);
  vv_builtin_type_int             = cast_to(type::integer);
  vv_builtin_type_nil             = cast_to(type::nil);
  vv_builtin_type_range           = cast_to(type::range);
  vv_builtin_type_regex           = cast_to(type::regex);
  vv_builtin_type_string          = cast_to(type::string);
  vv_builtin_type_string_iterator = cast_to(type::string_iterator);
  vv_builtin_type_symbol          = cast_to(type::symbol);
}

// }}}

void builtin::make_base_env(gc::managed_ptr base)
{
  value::get<vm::environment>(base).members = {
    { {"print"},          builtin::function::print },
    { {"puts"},           builtin::function::puts },
    { {"gets"},           builtin::function::gets },
    { {"count"},          builtin::function::count },
    { {"filter"},         builtin::function::filter },
    { {"map"},            builtin::function::map },
    { {"reduce"},         builtin::function::reduce },
    { {"sort"},           builtin::function::sort },
    { {"any"},            builtin::function::any },
    { {"all"},            builtin::function::all },
    { {"quit"},           builtin::function::quit },
    { {"reverse"},        builtin::function::reverse },
    { {"Array"},          builtin::type::array },
    { {"ArrayIterator"},  builtin::type::array_iterator },
    { {"Bool"},           builtin::type::boolean },
    { {"Dictionary"},     builtin::type::dictionary },
    { {"File"},           builtin::type::file },
    { {"Float"},          builtin::type::floating_point },
    { {"Integer"},        builtin::type::integer },
    { {"Nil"},            builtin::type::nil },
    { {"Object"},         builtin::type::object },
    { {"Range"},          builtin::type::range },
    { {"RegEx"},          builtin::type::regex },
    { {"RegExResult"},    builtin::type::regex_result },
    { {"String"},         builtin::type::string },
    { {"StringIterator"}, builtin::type::string_iterator },
    { {"Symbol"},         builtin::type::symbol },
    { {"Type"},           builtin::type::custom_type }
  };
}
