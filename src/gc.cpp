#include "gc.h"

using namespace vv;
using namespace gc;
using namespace internal;

value::nil gc::internal::g_nil{};
value::boolean gc::internal::g_true{true};
value::boolean gc::internal::g_false{false};
std::array<value::integer, 1024> gc::internal::g_ints;

namespace {

using gc_chunk = std::array<value_type, 512>;
std::list<gc_chunk> g_vals( 4 );

}

vm::machine* g_vm;

value_type& value_type::operator=(value::array&& other)
{
  (&base)->~base();
  new (this) value::array{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::array_iterator&& other)
{
  (&base)->~base();
  new (this) value::array_iterator{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::base&& other)
{
  (&base)->~base();
  new (this) value::base{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::boolean&& other)
{
  (&base)->~base();
  new (this) value::boolean{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::builtin_function&& other)
{
  (&base)->~base();
  new (this) value::builtin_function{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::dictionary&& other)
{
  (&base)->~base();
  new (this) value::dictionary{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::file&& other)
{
  (&base)->~base();
  new (this) value::file{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::floating_point&& other)
{
  (&base)->~base();
  new (this) value::floating_point{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::function&& other)
{
  (&base)->~base();
  new (this) value::function{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::integer&& other)
{
  (&base)->~base();
  new (this) value::integer{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::nil&& other)
{
  (&base)->~base();
  new (this) value::nil{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::range&& other)
{
  (&base)->~base();
  new (this) value::range{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::string&& other)
{
  (&base)->~base();
  new (this) value::string{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::string_iterator&& other)
{
  (&base)->~base();
  new (this) value::string_iterator{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::symbol&& other)
{
  (&base)->~base();
  new (this) value::symbol{std::move(other)};
  return *this;
}
value_type& value_type::operator=(value::type&& other)
{
  (&base)->~base();
  new (this) value::type{std::move(other)};
  return *this;
}
value_type& value_type::operator=(vm::environment&& other)
{
  (&base)->~base();
  new (this) vm::environment{std::move(other)};
  return *this;
}

void get_next(std::list<gc_chunk>::iterator& major, value_type*& minor)
{
  while (major != end(g_vals)) {
    minor = std::find_if(minor, end(*major),
                         [](const auto& i)
                           { return i.base.type == &builtin::type::nil; });
    if (minor != end(*major))
      break;
    ++major;
    minor = begin(*major);
  }
}

value_type* gc::internal::get_next_empty()
{
  static auto major = begin(g_vals);
  static auto minor = begin(*major);

  if (major == end(g_vals)) {
    mark();
    sweep();
    major = begin(g_vals);
    minor = begin(*major);

    get_next(major, minor);
    if (major == end(g_vals)) {
      --major;
      g_vals.resize(g_vals.size() * 2);
      ++major;
      minor = begin(*major);
    }
  }

  auto ret = minor;

  // since minor is by definition empty at this point, *don't* include it in the
  // next-empty search
  ++minor;
  get_next(major, minor);

  assert(ret->base.type == &builtin::type::nil && "search failed\n");
  return ret;
}

void gc::internal::mark()
{
  if (g_vm)
    g_vm->mark();
}

void gc::internal::sweep()
{
  for (auto& i : g_vals) {
    for (auto& j : i) {
      if (j.marked()) {
        j.unmark();
      } else {
        j = value::nil{};
      }
    }
  }
}

void gc::set_running_vm(vm::machine& vm)
{
  g_vm = &vm;
}

void gc::init()
{
  int value = 0;
  for (auto& i : internal::g_ints)
    i.val = value++;
}

void gc::empty() { }
