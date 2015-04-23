#include "builtins.h"
#include "gc/alloc.h"
#include "utils/string_helpers.h"
#include "value/floating_point.h"
#include "value/string.h"
#include "value/symbol.h"

namespace std {
std::ostream& operator<<(std::ostream& stm, const vv::tag tag);
std::ostream& operator<<(std::ostream& stm, const vv::symbol sym);
}

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <numeric>

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
  case vv::tag::file: return stm << "file";
  case vv::tag::floating_point: return stm << "floating_point";
  case vv::tag::function: return stm << "function";
  case vv::tag::integer: return stm << "integer";
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

void check_int(const int orig)
{
  const auto ptr = vv::gc::alloc<vv::value::integer>( orig );
  BOOST_CHECK_EQUAL(ptr.tag(), vv::tag::integer);
  BOOST_CHECK_MESSAGE(ptr.type() == vv::builtin::type::integer,
                      "incorrect type for " << orig);

  const auto val = vv::value::get<vv::value::integer>(ptr);
  BOOST_CHECK_EQUAL(orig, val);
}

void check_bool(const bool orig)
{
  const auto ptr = vv::gc::alloc<vv::value::boolean>( orig );
  BOOST_CHECK_EQUAL(ptr.tag(), vv::tag::boolean);
  BOOST_CHECK_MESSAGE(ptr.type() == vv::builtin::type::boolean,
                      "incorrect type for " << orig);

  const auto val = vv::value::get<vv::value::boolean>(ptr);
  BOOST_CHECK_EQUAL(orig, val);
}

void check_float(const double orig)
{
  const auto ptr = vv::gc::alloc<vv::value::floating_point>( orig );
  BOOST_CHECK_EQUAL(ptr.tag(), vv::tag::floating_point);
  BOOST_CHECK_MESSAGE(ptr.type() == vv::builtin::type::floating_point,
                      "incorrect type for " << orig);

  const auto val = vv::value::get<vv::value::floating_point>(ptr);
  BOOST_CHECK_EQUAL(orig, val);
}

BOOST_AUTO_TEST_CASE(check_nil)
{
  const auto ptr = vv::gc::alloc<vv::value::nil>( );
  BOOST_CHECK_EQUAL(ptr.tag(), vv::tag::nil);
  BOOST_CHECK_MESSAGE(ptr.type() == vv::builtin::type::nil,
                      "incorrect type for nil");
}

void check_string(const std::string& orig)
{
  const auto ptr = vv::gc::alloc<vv::value::string>( orig );
  BOOST_CHECK_EQUAL(ptr.tag(), vv::tag::string);
  BOOST_CHECK_MESSAGE(ptr.type() == vv::builtin::type::string,
                      "incorrect type for \"" << vv::escape_chars(orig) << '"');

  const auto val = vv::value::get<vv::value::string>(ptr);
  BOOST_CHECK_EQUAL(orig, val);
}

BOOST_AUTO_TEST_CASE(check_default_string)
{
  const auto ptr = vv::gc::alloc<vv::value::string>( );
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::string>(ptr), "");
}

void check_symbol(const vv::symbol orig)
{
  const auto ptr = vv::gc::alloc<vv::value::symbol>( orig );
  BOOST_CHECK_EQUAL(ptr.tag(), vv::tag::symbol);
  BOOST_CHECK_MESSAGE(ptr.type() == vv::builtin::type::symbol,
                      "incorrect type for " << orig);

  const auto val = vv::value::get<vv::value::symbol>(ptr);
  BOOST_CHECK_EQUAL(orig, val);
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char** argv)
{
  vv::builtin::init();

  std::array<int32_t, 1002> ints;
  ints[0] = std::numeric_limits<int32_t>::min();
  ints[1] = std::numeric_limits<int32_t>::max();
  std::iota(begin(ints) + 2, end(ints), -500);

  BOOST_PARAM_TEST_CASE(&check_int, begin(ints), end(ints));

  const auto bools = { true, false };
  BOOST_PARAM_TEST_CASE(&check_bool, begin(bools), end(bools));

  std::array<double, 1002> doubles;
  doubles[0] = std::numeric_limits<double>::min();
  doubles[1] = std::numeric_limits<double>::max();
  std::iota(begin(doubles) + 2, end(doubles), -500.0);
  BOOST_PARAM_TEST_CASE(&check_float, begin(doubles), end(doubles));

  const auto strings = { "", "foo", "foo\"bar", "foo\\bar" };
  BOOST_PARAM_TEST_CASE(&check_string, begin(strings), end(strings));

  const auto symbols = { vv::symbol{""}, vv::symbol{"foo"} };
  BOOST_PARAM_TEST_CASE(&check_symbol, begin(symbols), end(symbols));

  return nullptr;
}
