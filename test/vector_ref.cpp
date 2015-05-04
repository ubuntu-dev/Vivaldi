#include "output.h"

#include "utils/vector_ref.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

#include <numeric>

void check_vector(const std::vector<int>& vec)
{
  vv::vector_ref<int> ref{vec};
  // Ensure we got the right data before trying to access it!
  BOOST_REQUIRE_EQUAL(ref.data(), vec.data());
  // Iterators and size
  BOOST_CHECK_EQUAL_COLLECTIONS(std::begin(ref), std::end(ref),
                                std::begin(vec), std::end(vec));
  // Check operator[]
  for (auto i = ref.size(); i--;)
    BOOST_CHECK_EQUAL(vec[i], ref[i]);

  if (ref.size() == 0)
    BOOST_CHECK(ref.empty());
  else
    BOOST_CHECK(!ref.empty());
}

BOOST_AUTO_TEST_CASE(check_default_ctor)
{
  vv::vector_ref<std::string> ref;
  BOOST_CHECK_EQUAL(ref.size(), 0);
  BOOST_CHECK(ref.empty());
  BOOST_CHECK_EQUAL(std::begin(ref), std::end(ref));
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char** argv)
{
  std::array<std::vector<int>, 3> vectors;
  vectors[0] = {};
  vectors[1] = { 0, 0, 0, 0 };
  vectors[2].resize(10000);
  std::iota(begin(vectors[2]), end(vectors[2]), -5000);

  boost::unit_test::framework::master_test_suite().add(
    BOOST_PARAM_TEST_CASE(&check_vector, begin(vectors), end(vectors)));

  return nullptr;
}
