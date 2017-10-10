#include "../src/utils/string_helpers.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

using namespace std::literals;

BOOST_AUTO_TEST_CASE(ltrim)
{
  const auto empty = ""s;
  const auto no_ws = "foobar"s;
  const auto r_ws = "foobar   \n\n\t  \t  \n "s;
  const auto l_ws = "   \n\n\t  \t  \n foobar"s;
  const auto only_ws = "   \n\n\t  \t  \n "s;

  BOOST_CHECK_EQUAL(vv::ltrim(empty), empty);
  BOOST_CHECK_EQUAL(vv::ltrim(no_ws), no_ws);
  BOOST_CHECK_EQUAL(vv::ltrim(r_ws), r_ws);
  BOOST_CHECK_EQUAL(vv::ltrim(l_ws), no_ws);
  BOOST_CHECK_EQUAL(vv::ltrim(only_ws), empty);
}

BOOST_AUTO_TEST_CASE(to_int)
{
  const auto zero = "0"s;
  const auto dec_eighteen = "18"s;
  const auto hex_eighteen = "0x12"s;
  const auto oct_eighteen = "022"s;
  const auto bin_eighteen = "0b10010"s;
  const auto large = "70000000000"s;
  BOOST_CHECK_EQUAL(vv::to_int(zero), 0);
  BOOST_CHECK_EQUAL(vv::to_int(dec_eighteen), 18);
  BOOST_CHECK_EQUAL(vv::to_int(hex_eighteen), 18);
  BOOST_CHECK_EQUAL(vv::to_int(oct_eighteen), 18);
  BOOST_CHECK_EQUAL(vv::to_int(bin_eighteen), 18);
  BOOST_CHECK_EQUAL(vv::to_int(large), 70000000000ll);
}

BOOST_AUTO_TEST_CASE(isnamechar)
{
  BOOST_CHECK(vv::isnamechar('a'));
  BOOST_CHECK(vv::isnamechar('b'));
  BOOST_CHECK(vv::isnamechar('c'));
  BOOST_CHECK(vv::isnamechar('d'));
  BOOST_CHECK(vv::isnamechar('e'));
  BOOST_CHECK(vv::isnamechar('f'));
  BOOST_CHECK(vv::isnamechar('g'));
  BOOST_CHECK(vv::isnamechar('h'));
  BOOST_CHECK(vv::isnamechar('i'));
  BOOST_CHECK(vv::isnamechar('j'));
  BOOST_CHECK(vv::isnamechar('k'));
  BOOST_CHECK(vv::isnamechar('l'));
  BOOST_CHECK(vv::isnamechar('m'));
  BOOST_CHECK(vv::isnamechar('n'));
  BOOST_CHECK(vv::isnamechar('o'));
  BOOST_CHECK(vv::isnamechar('p'));
  BOOST_CHECK(vv::isnamechar('q'));
  BOOST_CHECK(vv::isnamechar('r'));
  BOOST_CHECK(vv::isnamechar('s'));
  BOOST_CHECK(vv::isnamechar('t'));
  BOOST_CHECK(vv::isnamechar('u'));
  BOOST_CHECK(vv::isnamechar('v'));
  BOOST_CHECK(vv::isnamechar('w'));
  BOOST_CHECK(vv::isnamechar('x'));
  BOOST_CHECK(vv::isnamechar('y'));
  BOOST_CHECK(vv::isnamechar('z'));

  BOOST_CHECK(vv::isnamechar('A'));
  BOOST_CHECK(vv::isnamechar('B'));
  BOOST_CHECK(vv::isnamechar('C'));
  BOOST_CHECK(vv::isnamechar('D'));
  BOOST_CHECK(vv::isnamechar('E'));
  BOOST_CHECK(vv::isnamechar('F'));
  BOOST_CHECK(vv::isnamechar('G'));
  BOOST_CHECK(vv::isnamechar('H'));
  BOOST_CHECK(vv::isnamechar('I'));
  BOOST_CHECK(vv::isnamechar('J'));
  BOOST_CHECK(vv::isnamechar('K'));
  BOOST_CHECK(vv::isnamechar('L'));
  BOOST_CHECK(vv::isnamechar('M'));
  BOOST_CHECK(vv::isnamechar('N'));
  BOOST_CHECK(vv::isnamechar('O'));
  BOOST_CHECK(vv::isnamechar('P'));
  BOOST_CHECK(vv::isnamechar('Q'));
  BOOST_CHECK(vv::isnamechar('R'));
  BOOST_CHECK(vv::isnamechar('S'));
  BOOST_CHECK(vv::isnamechar('T'));
  BOOST_CHECK(vv::isnamechar('U'));
  BOOST_CHECK(vv::isnamechar('V'));
  BOOST_CHECK(vv::isnamechar('W'));
  BOOST_CHECK(vv::isnamechar('X'));
  BOOST_CHECK(vv::isnamechar('Y'));
  BOOST_CHECK(vv::isnamechar('Z'));

  BOOST_CHECK(vv::isnamechar('_'));

  BOOST_CHECK(vv::isnamechar('0'));
  BOOST_CHECK(vv::isnamechar('1'));
  BOOST_CHECK(vv::isnamechar('2'));
  BOOST_CHECK(vv::isnamechar('3'));
  BOOST_CHECK(vv::isnamechar('4'));
  BOOST_CHECK(vv::isnamechar('5'));
  BOOST_CHECK(vv::isnamechar('6'));
  BOOST_CHECK(vv::isnamechar('7'));
  BOOST_CHECK(vv::isnamechar('8'));
  BOOST_CHECK(vv::isnamechar('9'));

  BOOST_CHECK(vv::isnamechar('!'));
  BOOST_CHECK(vv::isnamechar('?'));

  BOOST_CHECK(!vv::isnamechar(' '));
  BOOST_CHECK(!vv::isnamechar('\n'));
  BOOST_CHECK(!vv::isnamechar('\t'));

  BOOST_CHECK(!vv::isnamechar('\''));
  BOOST_CHECK(!vv::isnamechar('-'));
}

BOOST_AUTO_TEST_CASE(isoctdigit)
{
  BOOST_CHECK(vv::isoctdigit('0'));
  BOOST_CHECK(vv::isoctdigit('1'));
  BOOST_CHECK(vv::isoctdigit('2'));
  BOOST_CHECK(vv::isoctdigit('3'));
  BOOST_CHECK(vv::isoctdigit('4'));
  BOOST_CHECK(vv::isoctdigit('5'));
  BOOST_CHECK(vv::isoctdigit('6'));
  BOOST_CHECK(vv::isoctdigit('7'));

  BOOST_CHECK(!vv::isoctdigit('8'));
  BOOST_CHECK(!vv::isoctdigit('9'));

  BOOST_CHECK(!vv::isoctdigit('a'));
  BOOST_CHECK(!vv::isoctdigit('A'));
  BOOST_CHECK(!vv::isoctdigit(' '));
  BOOST_CHECK(!vv::isoctdigit('\n'));
  BOOST_CHECK(!vv::isoctdigit('_'));
  BOOST_CHECK(!vv::isoctdigit('-'));
  BOOST_CHECK(!vv::isoctdigit('+'));
}

BOOST_AUTO_TEST_CASE(escape_chars)
{
  const auto none = "foobar"s;
  const auto alarm = "foo\abar"s;
  const auto backspace = "foo\bbar"s;
  const auto newline = "foo\nbar"s;
  const auto page = "foo\fbar"s;
  const auto ret = "foo\rbar"s;
  const auto vtab = "foo\vbar"s;
  const auto quote = "foo\"bar"s;
  const auto backslash = "foo\\bar"s;
  const auto null = "foo\0bar"s;
  const auto low_nonprinting = "foo\023bar"s;
  const auto high_nonprinting = "foo\200bar"s;

  BOOST_CHECK_EQUAL(vv::escape_chars(none), "foobar");
  BOOST_CHECK_EQUAL(vv::escape_chars(alarm), "foo\\abar");
  BOOST_CHECK_EQUAL(vv::escape_chars(backspace), "foo\\bbar");
  BOOST_CHECK_EQUAL(vv::escape_chars(newline), "foo\\nbar");
  BOOST_CHECK_EQUAL(vv::escape_chars(page), "foo\\fbar");
  BOOST_CHECK_EQUAL(vv::escape_chars(ret), "foo\\rbar");
  BOOST_CHECK_EQUAL(vv::escape_chars(vtab), "foo\\vbar");
  BOOST_CHECK_EQUAL(vv::escape_chars(quote), "foo\\\"bar");
  BOOST_CHECK_EQUAL(vv::escape_chars(backslash), "foo\\\\bar");
  BOOST_CHECK_EQUAL(vv::escape_chars(null), "foo\\000bar");
  BOOST_CHECK_EQUAL(vv::escape_chars(low_nonprinting), "foo\\023bar");
  BOOST_CHECK_EQUAL(vv::escape_chars(high_nonprinting), "foo\\200bar");
}

BOOST_AUTO_TEST_CASE(get_escaped_name)
{
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\a'), "\\alarm");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\b'), "\\backspace");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\n'), "\\newline");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\f'), "\\page");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\r'), "\\return");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\t'), "\\tab");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\v'), "\\vtab");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('\0'), "\\nul");
  BOOST_CHECK_EQUAL(vv::get_escaped_name(' '),  "\\space");

  BOOST_CHECK_EQUAL(vv::get_escaped_name('a'), "\\a");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('b'), "\\b");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('c'), "\\c");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('d'), "\\d");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('e'), "\\e");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('f'), "\\f");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('g'), "\\g");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('h'), "\\h");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('i'), "\\i");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('j'), "\\j");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('k'), "\\k");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('l'), "\\l");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('m'), "\\m");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('n'), "\\n");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('o'), "\\o");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('p'), "\\p");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('q'), "\\q");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('r'), "\\r");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('s'), "\\s");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('t'), "\\t");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('u'), "\\u");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('v'), "\\v");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('w'), "\\w");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('x'), "\\x");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('y'), "\\y");
  BOOST_CHECK_EQUAL(vv::get_escaped_name('z'), "\\z");
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char** argv)
{
  return nullptr;
}
