#include "gc.h"

#include "builtins.h"

using namespace vv;


value::nil gc::internal::g_nil{};
value::boolean gc::internal::g_true{true};
value::boolean gc::internal::g_false{false};
std::array<value::integer, 1024> gc::internal::g_ints;

namespace {

std::vector<value::base*> g_vals;
vm::machine* g_vm;

void mark()
{
  if (g_vm)
    g_vm->mark();
}

void sweep()
{
  for (auto*& i : g_vals) {
    if (i->marked()) {
      i->unmark();
    } else {
      delete i;
      i = nullptr;
    }
  }

  g_vals.erase(remove(begin(g_vals), end(g_vals), nullptr), end(g_vals));
}

}

value::base* gc::internal::emplace(value::base* val)
{
  if (g_vals.capacity() == g_vals.size()) {
    mark();
    sweep();
    if (g_vals.capacity() == g_vals.size())
      g_vals.reserve(g_vals.capacity() * 2);
  }

  val->unmark();
  g_vals.push_back(val);
  return val;
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
  g_vals.reserve(512);
}

void gc::empty()
{
  for (auto i : g_vals)
    delete i;
  g_vals.clear();
}
