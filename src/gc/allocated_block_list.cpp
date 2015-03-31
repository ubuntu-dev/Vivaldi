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

void allocated_block_list::erase_destruct(iterator iter)
{
  destruct(**iter);
  m_data.erase(iter);
}

void allocated_block_list::erase_destruct(value_type ptr)
{
  erase_destruct(find(ptr));
}

allocated_block_list::~allocated_block_list()
{
  while (!empty())
    erase_destruct(begin());
}
