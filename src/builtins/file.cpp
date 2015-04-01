#include "builtins.h"

#include "gc/alloc.h"
#include "messages.h"
#include "vm.h"
#include "utils/lang.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/file.h"
#include "value/opt_functions.h"
#include "value/string.h"
#include "value/type.h"

#include <sstream>

using namespace vv;
using namespace builtin;

namespace {

value::object* fn_file_init(vm::machine& vm)
{
  vm.arg(0);
  auto arg = vm.top();
  if (arg->type != &type::string) {
    return throw_exception(message::init_type_error(type::file,
                                                    type::string,
                                                    *arg->type));
  }
  vm.self();
  auto self = static_cast<value::file*>(vm.top());
  const auto& filename = static_cast<value::string&>(*arg).val;
  self->val = std::fstream{filename};
  self->name = filename;
  std::getline(self->val, self->cur_line);
  return self;
}

value::object* fn_file_contents(value::object* self)
{
  auto& file = static_cast<value::file&>(*self);
  std::ostringstream str_stream;
  str_stream << file.cur_line;
  str_stream << file.val.rdbuf();
  file.cur_line.clear();
  return gc::alloc<value::string>( str_stream.str() );
}

value::object* fn_file_start(value::object* self)
{
  return self;
}

value::object* fn_file_get(value::object* self)
{
  const auto& file = static_cast<value::file&>(*self);
  return gc::alloc<value::string>( file.cur_line );
}

value::object* fn_file_increment(value::object* self)
{
  auto& file = static_cast<value::file&>(*self);
  if (file.val.peek() == EOF) {
    if (!file.cur_line.size())
      return throw_exception(message::iterator_at_end(type::file));
    file.cur_line.clear();
    return self;
  }
  std::getline(file.val, file.cur_line);
  return self;
}

value::object* fn_file_at_end(value::object* self)
{
  auto& file = static_cast<value::file&>(*self);
  return gc::alloc<value::boolean>(file.val.peek() == EOF && !file.cur_line.size());
}

value::builtin_function file_init {fn_file_init, 1};
value::opt_monop file_contents  {fn_file_contents };
value::opt_monop file_start     {fn_file_start    };
value::opt_monop file_get       {fn_file_get      };
value::opt_monop file_increment {fn_file_increment};
value::opt_monop file_at_end    {fn_file_at_end   };

}

vv::value::type vv::builtin::type::file {gc::alloc<value::file>, {
  { {"init"},      &file_init      },
  { {"contents"},  &file_contents  },
  { {"start"},     &file_start     },
  { {"get"},       &file_get       },
  { {"increment"}, &file_increment },
  { {"at_end"},    &file_at_end    }
}, vv::builtin::type::object, {"File"}};
