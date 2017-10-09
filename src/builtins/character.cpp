#include "character.h"

#include "value.h"
#include "gc/alloc.h"
#include "value/string.h"

using namespace vv;
using namespace builtin;

gc::managed_ptr character::ord(gc::managed_ptr self)
{
  const auto chr = value::get<value::character>(self);
  return gc::alloc<value::integer>( static_cast<value::integer>(chr) );
}

gc::managed_ptr character::to_str(gc::managed_ptr self)
{
  const auto chr = value::get<value::character>(self);
  return gc::alloc<value::string>( std::string{chr} );
}
