#ifndef VV_TEST_INPUT_H
#define VV_TEST_INPUT_H

#include "symbol.h"
#include "gc/managed_ptr.h"

#include <iostream>

namespace std {
std::ostream& operator<<(std::ostream& stm, const vv::tag tag);
std::ostream& operator<<(std::ostream& stm, const vv::symbol sym);
}

std::ostream& std::operator<<(std::ostream& stm, const vv::tag tag)
{
  switch (tag) {
  case vv::tag::nil: return stm << "nil";
  case vv::tag::array: return stm << "array";
  case vv::tag::array_iterator: return stm << "array_iterator";
  case vv::tag::blob: return stm << "blob";
  case vv::tag::boolean: return stm << "boolean";
  case vv::tag::builtin_function: return stm << "builtin_function";
  case vv::tag::character: return stm << "character";
  case vv::tag::dictionary: return stm << "dictionary";
  case vv::tag::exception: return stm << "exception";
  case vv::tag::file: return stm << "file";
  case vv::tag::floating_point: return stm << "floating_point";
  case vv::tag::function: return stm << "function";
  case vv::tag::integer: return stm << "integer";
  case vv::tag::method: return stm << "method";
  case vv::tag::object: return stm << "object";
  case vv::tag::opt_monop: return stm << "opt_monop";
  case vv::tag::opt_binop: return stm << "opt_binop";
  case vv::tag::partial_function: return stm << "partial_function";
  case vv::tag::range: return stm << "range";
  case vv::tag::regex: return stm << "regex";
  case vv::tag::regex_result: return stm << "regex_result";
  case vv::tag::string: return stm << "string";
  case vv::tag::string_iterator: return stm << "string_iterator";
  case vv::tag::symbol: return stm << "symbol";
  case vv::tag::type: return stm << "type";
  case vv::tag::environment: return stm << "environment";
  }
}

std::ostream& std::operator<<(std::ostream& stm, const vv::symbol sym)
{
  return stm << '\'' << to_string(sym);
}

#endif
