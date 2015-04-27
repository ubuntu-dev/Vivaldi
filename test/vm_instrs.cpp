#include "output.h"

#include "builtins.h"
#include "value.h"
#include "vm.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <numeric>

void check_pint(const int orig)
{
  vv::vm::machine vm{vv::vm::call_frame{}};

  vm.pint(orig);
  const auto val = vm.top();
  BOOST_CHECK_EQUAL(val.tag(), vv::tag::integer);
  BOOST_CHECK(val.type() == vv::builtin::type::integer);
  BOOST_CHECK_EQUAL(vv::value::get<vv::value::integer>(val), orig);
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
  return nullptr;
}
