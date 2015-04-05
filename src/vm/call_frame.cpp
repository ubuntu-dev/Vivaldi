#include "call_frame.h"

#include "builtins.h"
#include "gc.h"
#include "gc/alloc.h"

using namespace vv;

vm::environment::environment(gc::managed_ptr enclosing, gc::managed_ptr self)
  : basic_object {builtin::type::object},
    value        {enclosing, self}
{ }

vm::environment::value_type::value_type(gc::managed_ptr enclosing, gc::managed_ptr self)
  : enclosing {enclosing},
    self      {(self || !enclosing) ? self : value::get<environment>(enclosing).self},
    members   {}
{ }

vm::call_frame::call_frame(vector_ref<vm::command> instr_ptr,
                           gc::managed_ptr enclosing,
                           gc::managed_ptr self,
                           size_t argc,
                           size_t frame_ptr)
  : argc       {argc},
    frame_ptr  {frame_ptr},
    caller     {},
    catcher    {},
    instr_ptr  {instr_ptr},
    m_env      {enclosing, self},
    m_heap_env {}
{ }

vm::environment::value_type& vm::call_frame::env()
{
  return m_heap_env ? value::get<environment>(m_heap_env) : m_env;
}

const vm::environment::value_type& vm::call_frame::env() const
{
  return m_heap_env ? value::get<environment>(m_heap_env) : m_env;
}

void vm::call_frame::set_env(gc::managed_ptr env)
{
  m_heap_env = env;
}

gc::managed_ptr vm::call_frame::env_ptr()
{
  if (!m_heap_env) {
    m_heap_env = gc::alloc<environment>( m_env.enclosing, m_env.self );
    value::get<environment>(m_heap_env).members = std::move(m_env.members);
  }
  return m_heap_env;
}

void vm::call_frame::mark_env()
{
  if (m_heap_env) {
    gc::mark(m_heap_env);
  }
  else {
    gc::mark(m_env.enclosing);
    gc::mark(m_env.self);
    for (auto i : m_env.members)
      gc::mark(i.second);
  }
}
