#include "allocated_block_list.h"

#include "value/array.h"
#include "value/array_iterator.h"
#include "value/blob.h"
#include "value/boolean.h"
#include "value/builtin_function.h"
#include "value/dictionary.h"
#include "value/file.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/integer.h"
#include "value/nil.h"
#include "value/opt_functions.h"
#include "value/range.h"
#include "value/regex.h"
#include "value/string.h"
#include "value/string_iterator.h"
#include "value/symbol.h"

using namespace vv;
using namespace gc;

size_t gc::size_for(tag type)
{
  switch (type) {
  case tag::object:           return sizeof(value::object);
  case tag::array:            return sizeof(value::array);
  case tag::array_iterator:   return sizeof(value::array_iterator);
  case tag::blob:             return sizeof(value::blob);
  case tag::boolean:          return sizeof(value::boolean);
  case tag::builtin_function: return sizeof(value::builtin_function);
  case tag::dictionary:       return sizeof(value::dictionary);
  case tag::file:             return sizeof(value::file);
  case tag::floating_point:   return sizeof(value::floating_point);
  case tag::function:         return sizeof(value::function);
  case tag::integer:          return sizeof(value::integer);
  case tag::nil:              return sizeof(value::nil);
  case tag::opt_monop:        return sizeof(value::opt_monop);
  case tag::opt_binop:        return sizeof(value::opt_binop);
  case tag::range:            return sizeof(value::range);
  case tag::regex:            return sizeof(value::regex);
  case tag::regex_result:     return sizeof(value::regex_result);
  case tag::string:           return sizeof(value::string);
  case tag::string_iterator:  return sizeof(value::string_iterator);
  case tag::symbol:           return sizeof(value::symbol);
  case tag::type:             return sizeof(value::type);
  case tag::environment:      return sizeof(vm::environment);
  }
}

allocated_block_list::value_type allocated_block_list::erase(iterator iter)
{
  auto val = *iter;
  m_data.erase(iter);
  return val;
}

allocated_block_list::value_type allocated_block_list::erase(value_type ptr)
{
  m_data.erase(ptr);
  return ptr;
}

allocated_block_list::value_type allocated_block_list::erase(value::object* ptr)
{
  value_type val{ptr, tag::object};
  // get appropriate tag
  auto iter = find(val);
  val = *iter;
  m_data.erase(iter);
  return val;
}

free_block_list::value_type allocated_block_list::erase_destruct(iterator iter)
{
  iter->ptr->~object();
  auto ptr = reinterpret_cast<char*>(iter->ptr);
  auto tag = iter->tag;
  m_data.erase(iter);

  return {ptr, size_for(tag)};
}

free_block_list::value_type allocated_block_list::erase_destruct(value_type ptr)
{
  return erase_destruct(find(ptr));
}

free_block_list::value_type allocated_block_list::erase_destruct(value::object* ptr)
{
  return erase_destruct(find(ptr));
}

allocated_block_list::~allocated_block_list()
{
  while (!empty())
    erase_destruct(begin());
}
