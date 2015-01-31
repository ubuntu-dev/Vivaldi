#include "environment.h"

il::environment::environment(
    const std::unordered_map<symbol, value::base*>& local)
  : m_local_env {local},
    m_parent    {nullptr}
{ }

il::environment::environment(environment& prev)
  : m_parent {&prev}
{
  m_parent->m_children.push_back(this);
}

il::value::base*& il::environment::at(symbol name)
{
  if (m_local_env.count(name))
    return m_local_env[name];
  if (m_parent)
    return m_parent->at(name);
  throw std::runtime_error{"symbol '" + to_string(name) + " undefined"};
}

il::value::base* il::environment::assign(symbol name, value::base* val)
{
  return m_local_env[name] = val;
}

void il::environment::mark()
{
  for (const auto& i : m_local_env)
    if (!i.second->marked())
      i.second->mark();
  for (auto* i : m_children)
    i->mark();
}

il::environment::~environment()
{
  if (m_parent) {
    auto& children = m_parent->m_children;
    children.erase(find(begin(children), end(children), this));
  }
}
