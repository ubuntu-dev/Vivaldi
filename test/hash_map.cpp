#include "output.h"

#include "utils/hash_map.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

BOOST_AUTO_TEST_CASE(check_default_ctor)
{
  vv::hash_map<int, int> map;
  BOOST_CHECK_EQUAL(map.size(), 0);
  BOOST_CHECK(map.empty());
  BOOST_CHECK(std::begin(map) == std::end(map)); // can't print an iterator
  for (auto i = -100; i < 100; ++i)
    BOOST_CHECK_EQUAL(0, map.count(i));
}

BOOST_AUTO_TEST_CASE(check_at_operator)
{
  vv::hash_map<int, int> map;
  for (auto i = -500; i < 500; ++i)
    map[i] = i * 2;
  BOOST_CHECK(!map.empty());
  BOOST_CHECK_EQUAL(1000, map.size());
  for (auto i = -500; i < 500; ++i) {
    BOOST_CHECK_EQUAL(1, map.count(i));
    BOOST_CHECK_EQUAL(i * 2, map[i]);
    BOOST_CHECK(map.find(i) != std::end(map));
  }

  for (auto i = -500; i < 500; ++i)
    map[i] = i * 3;

  BOOST_CHECK_EQUAL(1000, map.size());
  for (auto i = -500; i < 500; ++i) {
    BOOST_CHECK_EQUAL(1, map.count(i));
    BOOST_CHECK_EQUAL(i * 3, map[i]);
  }
}

BOOST_AUTO_TEST_CASE(check_at_fn)
{
  vv::hash_map<int, int> map;
  for (auto i = -500; i < 500; ++i)
    map[i] = i * 2;
  for (auto i = -500; i < 500; ++i)
    BOOST_CHECK_EQUAL(map.at(i), i * 2);
  BOOST_CHECK_THROW(map.at(500), std::out_of_range);
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char** argv)
{
  return nullptr;
}
