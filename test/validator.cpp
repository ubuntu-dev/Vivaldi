#include "parser.h"
#include "utils/string_helpers.h"

#include <boost/test/included/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>

using namespace std::string_literals;

void check_if_valid(const char* i)
{
  BOOST_MESSAGE('"' << vv::escape_chars(i) << '"');
  std::stringstream str{};
  str << i;
  const auto tokenized = vv::parser::tokenize(str);
  BOOST_CHECK_MESSAGE(vv::parser::is_valid(tokenized),
                      '"' << vv::escape_chars(i) << "\""
                      " is incorrectly marked invalid");
}

void check_if_invalid(const char* i)
{
  std::stringstream str{};
  str << i;
  const auto tokenized = vv::parser::tokenize(str);
  BOOST_CHECK_MESSAGE(!vv::parser::is_valid(tokenized),
                      '"' << vv::escape_chars(i) << "\""
                      " is incorrectly marked valid");
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char** argv)
{
  const auto valid = {
    // Blank
    "",
    "//foo",
    "\n\n//foo\n",

    // Single-token expressions
    "1",
    "0xff87",
    "01775",
    "0b1011",
    "1.12",
    "\"foobar\"",
    "\"fo\\\"obar\"",
    "`foo`",
    "foo",
    "@foo",
    "true",
    "false",
    "nil",
    "\nfoo",
    "\nfoo\n",

    // Prefix operators
    "!foo",
    "!!foo",
    "!!!foo",
    "!!!!foo",
    "~foo",
    "~~foo",
    "~~~foo",
    "~~~~foo",
    "!~!~foo",
    "-foo",
    "--foo",
    "---foo",
    "----foo",
    "!~-!~-foo",

    "!\nfoo",
    "!\n\nfoo",
    "!\n!foo",
    "!\n!\nfoo",
    "!\n\n!\n\n!foo",
    "~\nfoo",
    "-\nfoo",

    // Postfix operators
    "foo[1]",
    "foo()",
    "foo(1)",
    "foo(1, 2)",
    "foo(1, 2, 3)",
    "foo[1]()",
    "foo(1, 2, 3)[4]",
    "foo()[1]()[2]()[3](4)(5)",

    // High-prec infix operators
    "foo.bar",
    "1.bar",
    "foo->bar",
    "foo->bar->baz",
    "foo.bar.baz",
    "foo->bar.baz->qux.blah",

    // Low-prec infix operators
    "1 + 2",
    "foo + bar",
    "foo - bar",
    "foo * bar",
    "foo / bar",
    "foo % bar",
    "foo ** bar",
    "foo & bar",
    "foo | bar",
    "foo ^ bar",
    "foo == bar",
    "foo != bar",
    "foo < bar",
    "foo > bar",
    "foo <= bar",
    "foo >= bar",
    "foo to bar",
    "foo && bar",
    "foo || bar",

    "foo + bar + 3 + baz + nil + qux",

    "foo + bar - 3 * baz / nil % qux",
    "foo ** bar & 3 | baz ^ nil && qux || blah",

    "foo + \n\n\n bar",
    "foo && \n\n\n bar",

    // Array literals
    "[]",
    "[\n\n\n\n]",
    "[1]",
    "[\n\n1\n\n]",
    "[1, 2, 3, 4, 5]",
    "[[[[[], 1, []], [[]]]]]",
    "[\n1,\n2,\n3\n]",

    // Blocks
    "do end",
    "do\n\n\nend",
    "do;;;end",
    "do 1\n2\n3 end",
    "do\n\n1;;;2;3;4\n\n5;1 + 12;[\n]\n\nend",
    "do do 5; 3 end\nend",

    // Dict literals
    "{}",
    "{\n\n}",
    "{foo: bar}",
    "{'foo: 'bar, 1: 2, nil: 5, 1.0: 45.0, []: 12, {}: {{}: {}}}",
    "{\n\n\n1:\n\n2,\n\nfoo:\n\nbar\n\n}",

    // Except
    "throw 1",
    "throw 1 + 23",
    "throw bar",

    // For loops
    "for i in 1 to 10: puts(i)",
    "for i in nil: 1",
    "for i in nil:\n\n\n\nputs(i) + 12",

    // Function definitions
    "let foo() = nil",
    "let foo(a, b, c, d, e, f, g, h, i, j, k, l) = a + b + c - d",
    "let foo(\na,\nb,\nc\n) =\nnil",

    // Lambdas
    "fn (): nil",
    "fn (\n\n\n):\n\n\nnil",
    "fn (a, b, c, d, e, f, g, h, i, j, k, l): a + b + c - d",
    "fn (\na,\nb,\nc\n):\nnil",

    // Member assignment
    "@foo = bar",
    "@foo = bar + baz",
    "@foo =\n\n\n\n\nbaz",

    // Object creation
    "new Foo()",
    "new Foo(bar)",
    "new Foo(bar, baz, 1, 2, [], {{}:{}})",
    "new (1 + 2 + 3 + bar + baz / nil)(bar)",
    "new Foo(\n\n\n)",
    "new Foo(\n\nbar\n\n)",
    "new Foo(\n\nbar,\n\nbaz\n\n)",

    // Requires
    "require \"foobar\"",

    // Returns
    "return nil",
    "return foo + bar",

    // Try/catch blocks
    "try: 5 catch Exception e: 6",
    "try:\n\n\n5\n\n\ncatch Integer e:\n\n\n6",
    "try:\n\n\nbar + baz\n\n\ncatch Foo e:\n\n\n6 + qux",
    "try: 5 catch Exception e: 6, RuntimeError r: 12",
    "try: 5 catch Exception e: 6,\n\n\nRuntimeError r: 12",

    // Type definitions
    "class Foo end",
    "class Foo : Bar end",
    "class Foo\n\n\nend",
    "class Foo : Bar\n\n\nend",
    "class Foo let foo(x) = x end",
    "class Foo : Bar let foo(x) = x end",
    "class Foo\n\n\n let foo(x) =\n\n\nx\n\n\nend",
    "class Foo\n let foo(x) = x\n let bar(x) = x + 12\nend",

    // Variable assignment
    "foo = 1",
    "foo = bar",
    "foo = bar + baz",
    "foo =\n\n\nbar",

    // While loops
    "while nil: nil",
    "while 1 < 12: i = i + 45",
    "while nil:\n\n\nnil"
  };

  const auto invalid = {
    // Blank
    "()",
    "(",
    ")",

    // Single-token expressions
    "1.",
    ".1",
    "09",
    "0b12",
    "\"foo\"bar\"",
    "\"foo\\\"",
    "'12",
    "@12",

    // Prefix operators
    "!",
    "-",
    "~",
    "!-~",
    "!\n",
    "!\n~\n-\n",

    // Postfix operators
    "a[]",
    "a[",
    "a]",
    "a\n(b, c)",
    "a(b\n,c)",
    "a)",
    "a(",

    // High-prec infix operators
    "a.",
    "a\n.b",
    ".a",
    ".\na",
    "a->",
    "a\n->b",
    "->a",
    "->\na",
    "a..b",
    "a->->b",
    "a.->b",

    // Low-prec infix operators
    "foo +",
    "foo -",
    "foo *",
    "foo /",
    "foo %",
    "foo **",
    "foo &",
    "foo |",
    "foo ^",
    "foo ==",
    "foo !=",
    "foo <",
    "foo >",
    "foo <=",
    "foo >=",
    "foo to",
    "foo &&",
    "foo ||",

    "+ foo", // no '-', for obvious reasons
    "* foo",
    "/ foo",
    "% foo",
    "** foo",
    "& foo",
    "| foo",
    "^ foo",
    "== foo",
    "!= foo",
    "< foo",
    "> foo",
    "<= foo",
    ">= foo",
    "to foo",
    "&& foo",
    "|| foo",

    "foo\n+ bar",
    "foo + + bar",
    "foo & & bar",
    "foo | | bar",

    // Array literals
    "[",
    "]",
    "[a\nb]",
    "[a\n,b'",
    "[][]",

    // Block literals
    "do",
    "end",
    "do 1",
    "do 1 2 end",

    // Cond literals
    "cond",
    "cond\n\n",
    "cond 1",
    "cond 1: 2 3: 3",
    "cond 1\n:2",

    // Dict literals
    "{",
    "}",
    "{a}",
    "{a:}",
    "{:a}",
    "{a:b,}",
    "{a:b\n,c:d}",
    "{a\n:b}",

    // Throw
    "throw",
    "throw\nfoo",
    "except foo",

    // For loops
    "for i in a b",
    "for\ni in a: b",
    "for i\n in a: b",
    "for i in\n a: b",
    "for i in a\n: b",
    "for i in a:",

    // Function definitions
    "let a()\n = b",
    "let a() b",
    "let a( = b",
    "let a) = b",
    "let\na() = b",
    "a() = b",


    // Lambdas
    "fn a: b",
    "fn a(): b",
    "fn a() b",
    "fn () b",
    "fn : b",
    "fn a():",
    "fn\na(): b",
    "fn a\n(): b",
    "fn ()\n: b",
    "fn () = b",

    // Member assignment
    "@a\n= b",
    "@a =",
    "let @a = b",

    // Object creation
    "new Foo",
    "new 1 + 2(3)",
    "new\nFoo()",
    "new Foo\n()",
    "new Foo(a,)",
    "new Foo)",
    "new Foo(",

    // Requires
    "require",
    "require\n\"foo\"",
    "require foo",

    // Returns
    "return",
    "return\nfoo",

    // Try/catch blocks
    "try",
    "try: a",
    "try a catch b: c",
    "try: a catch b c",
    "try: a catch: b",
    "try: catch a: b",
    "try: a catch b:",
    "try\n: a catch b: c",
    "try: a catch\nb: c",
    "try: a catch b\n: c",
    "try: a catch b: c",
    "try: a catch Type b: c\n, OtherType d: e",
    "try: a catch Type b: c,;;; OtherType d: e",

    // Type definitions
    "class",
    "class end",
    "class Foo : Bar",
    "class : Bar end",
    "class\nFoo end",
    "class Foo\n: Bar end",
    "class Foo :\nBar end",
    "class Foo fn foo(): nil end",
    "class Foo fn (): nil end",
    "class Foo foo(): nil end",
    "class Foo foo() = nil end",
    "class Foo let foo(): nil end",
    "class Foo let foo = nil end",
    "class Foo let foo()\n= nil end",
    "class Foo let foo() = nil let bar() = nil end",

    // Variable assignments
    "a =",
    "a\n= 3",

    // Variable declarations
    "let",
    "let a",
    "let a 3",
    "let a =",
    "let\na = 3",
    "let a\n= 3"
  };

  boost::unit_test::framework::master_test_suite().add(
      BOOST_PARAM_TEST_CASE(&check_if_valid, begin(valid), end(valid)));

  boost::unit_test::framework::master_test_suite().add(
      BOOST_PARAM_TEST_CASE(&check_if_invalid, begin(invalid), end(invalid)));
  return nullptr;
}
