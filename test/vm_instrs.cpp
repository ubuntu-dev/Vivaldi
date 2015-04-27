#include "output.h"

#include "builtins.h"
#include "value.h"
#include "vm.h"
#include "value/array.h"
#include "value/floating_point.h"
#include "value/string.h"
#include "value/symbol.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <numeric>

BOOST_AUTO_TEST_CASE(check_pbool)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pbool(false);
  const auto false_v = vm.top();
  BOOST_CHECK_EQUAL(false_v.tag(), vv::tag::boolean);
  BOOST_CHECK(false_v.type() == vv::builtin::type::boolean);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::boolean>(false_v), false);

  vm.pbool(true);
  const auto true_v = vm.top();
  BOOST_CHECK_EQUAL(true_v.tag(), vv::tag::boolean);
  BOOST_CHECK(true_v.type() == vv::builtin::type::boolean);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::boolean>(true_v), true);
}

void check_pchar(const char orig)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pchar(orig);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::character);
  BOOST_CHECK(val.type() == vv::builtin::type::character);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::character>(val), orig);
}

void check_pflt(const double orig)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pflt(orig);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::floating_point);
  BOOST_CHECK(val.type() == vv::builtin::type::floating_point);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::floating_point>(val), orig);
}

void check_pint(const int orig)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pint(orig);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::integer);
  BOOST_CHECK(val.type() == vv::builtin::type::integer);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::integer>(val), orig);
}

BOOST_AUTO_TEST_CASE(check_pnil)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pnil();
  const auto nil = vm.top();
  BOOST_CHECK_EQUAL(nil.tag(), vv::tag::nil);
  BOOST_CHECK(nil.type() == vv::builtin::type::nil);
}

void check_pstr(const std::string& orig)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pstr(orig);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::string);
  BOOST_CHECK(val.type() == vv::builtin::type::string);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::string>(val), orig);
}

void check_psym(const vv::symbol orig)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.psym(orig);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::symbol);
  BOOST_CHECK(val.type() == vv::builtin::type::symbol);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::symbol>(val), orig);
}

BOOST_AUTO_TEST_CASE(check_parr)
{
  vv::vm::machine vm{vv::vm::call_frame{}};
  vm.parr(0);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::array);
  BOOST_CHECK(val.type() == vv::builtin::type::array);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::array>(val).size(), 0);

  vm.pop(1);
  vm.pstr("sentinel");
  for (auto i = 0; i != 10; ++i)
    vm.pint(i);
  vm.parr(10);
  const auto arr10 = vm.top();
  BOOST_CHECK_EQUAL(arr10.tag(), vv::tag::array);
  BOOST_CHECK(arr10.type() == vv::builtin::type::array);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::array>(arr10).size(), 10);
  for (auto i = 0; i != 10; ++i) {
    const auto mem = vv::value::get<vv::value::array>(arr10)[i];
    BOOST_CHECK_EQUAL(mem.tag(), vv::tag::integer);
    BOOST_CHECK_EQUAL(vv::value::get<vv::value::integer>(mem), i);
  }

  vm.pop(1);
  const auto sentinel = vm.top();
  BOOST_CHECK_EQUAL(sentinel.tag(), vv::tag::string);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::string>(sentinel), "sentinel");
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char** argv)
{
  vv::builtin::init();

  std::array<int32_t, 1002> ints;
  ints[0] = std::numeric_limits<int32_t>::min();
  ints[1] = std::numeric_limits<int32_t>::max();
  std::iota(begin(ints) + 2, end(ints), -500);

  boost::unit_test::framework::master_test_suite().add(
    BOOST_PARAM_TEST_CASE(&check_pint, begin(ints), end(ints)));

  std::array<char, 256> chars;
  std::iota(begin(chars), end(chars), '\0');

  boost::unit_test::framework::master_test_suite().add(
    BOOST_PARAM_TEST_CASE(&check_pchar, begin(chars), end(chars)));

  std::array<double, 1003> doubles;
  doubles[0] = std::numeric_limits<double>::min();
  doubles[1] = std::numeric_limits<double>::max();
  doubles[2] = 0.125;
  std::iota(begin(doubles) + 3, end(doubles), -500.0);

  boost::unit_test::framework::master_test_suite().add(
    BOOST_PARAM_TEST_CASE(&check_pflt, begin(doubles), end(doubles)));

  const auto strings = {
    "",
    "foo",
    "hello world",
    // I don't really know what there is to test for this one. Check weird
    // attempts at parsing, maybe?
    "do",
    "let",
    "fn",
    // Or maybe escaped characters?
    "foo\"bar",
    "foo\\\n\tbar"
  };

  boost::unit_test::framework::master_test_suite().add(
    BOOST_PARAM_TEST_CASE(&check_pstr, begin(strings), end(strings)));

  std::array<vv::symbol, 8> symbols {{
    {"foo"},
    {"bar"},
    {"do"},
    {"let"},
    {"foo bar baz"},
    // ensure repeats work correctly
    {"foo"},
    {""},
    {"Integer"}
  }};

  boost::unit_test::framework::master_test_suite().add(
    BOOST_PARAM_TEST_CASE(&check_psym, begin(symbols), end(symbols)));

  return nullptr;
}
