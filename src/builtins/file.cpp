#include "builtins/file.h"

#include "builtins.h"
#include "gc/alloc.h"
#include "messages.h"
#include "utils/lang.h"
#include "value/file.h"
#include "value/string.h"

#include <sstream>

using namespace vv;
using namespace builtin;

gc::managed_ptr file::init(vm::machine& vm)
{
  vm.arg(0);
  auto arg = vm.top();
  if (arg.tag() != tag::string) {
    return throw_exception(message::init_type_error(type::file,
                                                    type::string,
                                                    arg.type()));
  }

  vm.self();
  const auto self = vm.top();
  const auto& filename = value::get<value::string>(arg);

  value::get<value::file>(self).val = std::fstream{filename};
  value::get<value::file>(self).name = filename;
  std::getline(value::get<value::file>(self).val,
               value::get<value::file>(self).cur_line);
  return self;
}

gc::managed_ptr file::contents(gc::managed_ptr self)
{
  auto& file = value::get<value::file>(self);
  std::ostringstream str_stream;
  str_stream << file.cur_line;
  str_stream << file.val.rdbuf();
  file.cur_line.clear();
  return gc::alloc<value::string>( str_stream.str() );
}

gc::managed_ptr file::start(gc::managed_ptr self)
{
  return self;
}

gc::managed_ptr file::get(gc::managed_ptr self)
{
  const auto& file = value::get<value::file>(self);
  return gc::alloc<value::string>( file.cur_line );
}

gc::managed_ptr file::increment(gc::managed_ptr self)
{
  auto& file = value::get<value::file>(self);
  if (file.val.peek() == EOF) {
    if (file.cur_line.empty())
      return throw_exception(message::iterator_at_end(type::file));
    file.cur_line.clear();
    return self;
  }
  std::getline(file.val, file.cur_line);
  return self;
}

gc::managed_ptr file::at_end(gc::managed_ptr self)
{
  auto& file = value::get<value::file>(self);
  return gc::alloc<value::boolean>(file.val.peek() == EOF && !file.cur_line.size());
}
