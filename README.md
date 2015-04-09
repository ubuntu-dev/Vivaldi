# Vivaldi #

Vivaldi is a simple interpreted language inspired by Ruby, Python, Lisp, etc,
supporting duck-typing, object orientation, and some functional constructs.

## Overview ##

### Basics ###

Vivaldi can be run either from a file or from the REPL:

    $ cat test.vv
    puts("Hello, world!")
    $ vivaldi ./test.vv
    Hello, world!
    $ ./vivaldi
    >>> puts("Hello, REPL!")
    Hello, REPL!
    => nil
    >>> quit()
    $

Vivaldi expressions are separated by newlines or semicolons.
Comments in Vivaldi are C-style `// till end of line` comments&mdash; multiline
comments aren't supported yet. For a full description of the grammar in
Backus-Naur form, see grammar.txt.

### Compiling

* If you don't already have it, install boost (`brew install boost` on OS X;
  Linux might be trickier, for reasons explained below)
* If you don't have CMake 3.0 or above, install it (`brew install cmake`,
  `pacman -S cmake`, `sudo apt-get install cmake` for OS X, Arch Linux, and
  Ubuntu respectively)
* Ensure that clang++ and libc++ >= 3.5 are installed.

        $ git clone git@github.com:jeorgun/Vivaldi.git
        $ cd Vivaldi
        $ mkdir build
        $ cd build
        $ # To build locally
        $ cmake .. && make
        $ # To install to some directory
        $ cmake -DCMAKE_INSTALL_PREFIX=/my/install/directory
        $ make && make install

Vivaldi's been tested on 64-bit OS X 10.10.2, and 32-bit Arch Linux with Linux
3.18, both with Clang/libc++ 3.5 and Boost 1.57.0. libc++ is required, and,
unfortunately, since Boost binaries are used, so is a Boost compiled with
libc++. The codebase is (or should be!) is fully conforming C++14--- it's quite
easy to add support for libstdc++, since it would basically consist of ripping
out a bunch of C++14 features, but I'm not really inclined to do that unless
there's a particularly pressing need.

### Builtins ###

Vivaldi has a fairly limited set of builtin types:

#### Objects ####

The root of the inheritance tree; every type in Vivaldi inherits from Object
(which in turn inherits from... itself; it really is turtles all the way down!)
Objects support a few universal methods:

* `type()` &mdash; Returns the type of `self`.
* `member(x)`&mdash; Returns the member with the name `x` (where `x` is a
Symbol); if member `x` doesn't exist, throws an exception.
* `has_member(x)`&mdash; Returns true if `self` has a member named `x`, and
  false otherwise.
* `set_member(x, y)`&mdash; Sets the member with the name `x` (where `x` is a
  Symbol) to value `y`, overwriting member `x` if it already exists.
* `not()` &mdash;  Returns whether or not `self` is truthy (i.e. not `false` or
  `nil`).
* `equals(x)`, `unequal(x)` &mdash; Returns true if `self` has the same object
ID as `x`, and `false` otherwise (vice versa for `unequal`). These methods
should be overridden by any classes with value-based concepts of semantics (for
instance, in the standard library, `String`, `Symbol`, `Integer`, and so forth
all override them).

#### Nil ####
`nil` &mdash; like `nil` in Ruby and Lisp, or `None` in Python.

#### Bools ####
`true` and `false`. What'd you expect?

#### Floats ####
64-bit floating-point values.

#### Integers ####
32-bit signed integers. Integer literals can be written in decimal, hexadecimal,
octal, or binary:

    let eighteen = 18
    eighteen == 0x12
    eighteen == 022
    eighteen == 0b10010

* `sqrt()`&mdash; Returns the square root of `self`.
* `sin()`&mdash; Returns the sine of `self` in radians.
* `cos()`&mdash; Returns the cosine of `self` in radians.
* `tan()`&mdash; Returns the tangent of `self` in radians.
* `chr()`&mdash; Returns a String containing one character `x`, with the
  character value of `self`; `an_int.chr().ord()` should always equal `an_int`,
  unless it's out of the range [0, 256) (in which case it'll except).

In addition, all mathematical and bitwise (i.e. not indexing-related) operators
apply to Integers.

#### Strings ####
Simple, immutable string class. Currently supports:

* `init(x)`&mdash; if `x` is a String or a Symbol, copies its string value;
  otherwise, creates a String with `x`'s display value. For instance, `new
  String(12)` will return `"12"`.
* `size()`&mdash; returns the size of the string.
* `add(x)`&mdash; Returns a string formed form concatenating `self` and the String
  `x`, leaving `self` unchanged.
* `times(x)`&mdash; Returns a string formed by concatenating `x` copies of `self`,
  leaving `self` unchanged.
* `start()`&mdash; Returns an iterator pointing to the beginning of `self`.
* `stop()`&mdash; Returns an iterator pointing to the end of `self`.
* `to_upper()`&mdash; Returns a copy of `self` with all lowercase letters
  switched with their uppercase equivalents.
* `to_lower()`&mdash; Returns a copy of `self` with all uppercase letters
  switched with their lowercase equivalents.
* `starts_with(x)`&mdash; Returns `true` if `self` begins with the string `x`,
  and `false` otherwise.
* `ord()`&mdash; Returns the integer value of `self.at(0)` (unless `self` is
  empty, which results in an exception).
* `split(x)`&mdash; Returns an Array of each substrings of `self` separated by
  the String `x`.
* `replace(x, y)`&mdash; Returns a String with all occurrences of RegEx `x`
  replaced by String `y`.
* `equals(x)`, `unequal(x)`&mdash; Returns `true` if `x` is a String equal in value
  to `self`, and `false` otherwise (vice versa for `unequal`).
* `less(x)`, `greater(x)`, `less_equals(x)`, `greater_equals(x)`&mdash; Return
  the result of the appropriate lexographical comparsion between `self` and
  String `x`.

#### Symbols ####
`'symbol_name` - as in Ruby or Lisp:

* `init(x)`&mdash; Creates a new Symbol with the string value of `x`, where `x` is a
  String or a Symbol.
* `equals(x)`, `unequal(x)`&mdash; Returns `true` if `x` is a Symbol equal in value
  to `self`, and `false` otherwise (vice versa for `unequal`).

#### RegExes ####
Extremely basic regular expression class:

    let regex = `foo.*bar`
    regex.match_index("no match") // nil
    regex.match_index("this string contains 'foobazbar'") // 22

* `init(x)`&mdash; if `x` is a RegEx, creates a copy of `x`; if `x` is a String,
  returns a RegEx made by compiling `x`; excepts otherwise.
* `match(x)`&mdash; returns a RegExResult (see below) of the first match of
`self` in `x`.
* `match_index(x)`&mdash; If `self` matches any substring of `x`, returns the
  index of the first matching substring within `x` as an Integer; otherwise,
  returns `nil`.

RegExResult is a class for managing submatches. It can only be instantiated from
appropriate RegEx methods (only `match` at the moment):

* `size()`: Number of submatches, plus one for the complete match.
* `at(x)`: Returns the string value of the `x`th match, where 0 is the complete
  match. For instance,

        let res = `(foo)(bar)`.match("foobar")
        res[0] == "foobar"
        res[1] == "foo"
        res[2] == "bar"
* `index(x)`: Returns the starting index of the `x`th match in the string `self`
  was created from; for instance, in the above example, `res.index(2)` is `3`.

Vastly expanded functionality to come soon.

#### Arrays ####
Simple mutable array type:

    let array = [1, 2, 3]
    let three = array.size()
    array.append('foo)
    let four = array.size()
    let two = array[1]
    array[2] = "foo"
    array // [1, 2, "foo"]

* `init(x)`&mdash; Returns a copy of the Array `x`.
* `size()`&mdash; Returns the size of `self`.
* `append(x)`&mdash; Pushes the value `x` onto the end of `self`.
* `pop()`&mdash; Returns the last value in `self`, and removes it from `self`.
* `at(x)`&mdash; Returns the value at index `x`.
* `set_at(x, y)`&mdash; Sets the value at index `x` to `y`, returning `y`.
* `start()`&mdash; Returns an iterator pointing to the beginning of `self`.
* `stop()`&mdash; Returns an iterator pointing to the end of `self`.
* `add(x)`&mdash; Returns the concatenation of `self` and Array `x`, leaving `self`
  unchanged.
* `equals(x)`&mdash; Returns `true` if `x` is an Array with identical members to
  `self` (as compared by calling `a == b` for each corresponding member of
  `self` and `x` as `a` and `b` respectively), and `false` otherwise.
* `unequal(x)`&mdash; Returns `!(self == x)`.

#### Dictionaries ####
Mutable hash-map type. At the moment, there's no way to override a type's
equality or hash methods, so you're stuck with whatever you're inheriting
from&mdash; which is based on object ID if you derive from Object:

    let dict = { 'foo: 5, "bar": 6 }
    let two = dict.size()
    dict[0.5] = 'baz
    let three = dict.size()
    let five = dict['foo]
    dict // { 0.500000: 'baz, 'foo: 5, "bar": 6 }

* `init(x)`&mdash; Returns a copy of the Dictionary `x`.
* `size()`&mdash; Returns the size of `self`.
* `at(x)`&mdash; Returns the value at key `x`; if no such value exists, inserts (and
  returns) `nil` at `x`.
* `set_at(x, y)`&mdash; Sets the value at key `x` to `y`, returning `y`.

#### Ranges ####
Provides a range over any pair of objects that can be

* Compared with `>` (`greater`)
* Incremented by adding 1 (via `add`)

A range covers [start, end):

    >>> for i in 1 to 5: puts(i) // "<x> to <y>" is sugar for "new Range(x, y)"
    1
    2
    3
    4
    => nil

* `init(x, y)`&mdash; Returns a Range from `x` to `y`. If they're not comparable or
  incrementable, this won't blow up *immediately*&mdash; only when you first try to
  use it.
* `start()`&mdash; Just returns a copy of `self`; see the section on Iterators
  to understand why.
* `size()`&mdash; Returns `y - x`. Don't call this if subtraction won't work!
* `at_end()`&mdash; Returns if `x == y` (well, actually, if `!(y > x)`, so a Range
  from `1.3` to `5.0` doesn't go on infinitely).
* `increment()`&mdash; Add 1 to `x`
* `to_arr()`&mdash; Creates an Array from all values from `x` to `y`.

#### Files ####

File support right now is pretty minimal. Files are valid ranges (and iterators;
`start` just returns the file it's called on) over their contained lines:

    $ cat myfile.txt
    hello
    world
    $ ./vivaldi
    >>> let file = new File("myfile.txt")
    >>> for i in file: puts("line: " + i)
    line: hello
    line: world
    => nil
    >>> i.at_end()
    => true

Alternatively, the contents of the file can be read in one fell swoop:

    >>> file.contents()
    => "hello
    world"
    >>> file.at_end()
    => true

That's basically it for now; expanded methods, plus writing to files, will be
added later.

* `init(x)`&mdash; Creates a new File with the filename `x`, where `x` is a
String.
* `start()`&mdash; Returns `self` (see ranges for why this is needed).
* `get()`&mdash; Returns the current line `self` is pointing to.
* `increment()`&mdash; Discards the current line and grabs the next, returning
  `self`; excepts if `self` is already at the end of the file.
* `at_end()`&mdash; Returns `true` if `self` is at the end of the file, and
  `false` otherwise.
* `contents()`&mdash; Returns the full contents of the file from the current
  line to the end, incrementing `self` to the end of the file.

#### Functions ####
Functions! Syntactically, a function is very simple:

    fn <name>(<arguments>): <body>

A function body is any valid Vivaldi expression (which is to say any valid
Vivaldi code):

    fn is_three(x): x == 3

A lambda is identical to a function, just without the name&mdash; or, more
accurately, a function is just a lambda with a name. The following:

    let five_returner = fn(): 5

is completely identical to

    fn five_returner(): 5

Once defined, functions work more or less like in Python:

    fn id(x): x

    let function = id
    function(1) // 1

`return` can be used to exit out of a function early, but it's unnecessary for
the last expression of a function body (or *only* expression, unless it's
wrapped in a block):

    // contrived; this could just be 'fn is_even(x): x % 2 == 0'
    fn is_even(x): do
      if x % 2 == 0: return true
      false
    end

#### Types ####
Everything in Vivaldi is an object, and has

* Members (accessible within classes with the Ruby-alike prefix '@', and outside
  by the Object methods `member` and `set_member`)

        class Foo
          fn set_foo(x): @foo = x
          fn get_bar(): @bar
        end

        let a = new Foo()
        a.set_foo(5)
        let five = a.member('foo)
        a.set_member('bar, "hello")
        let hello = a.get_bar()

* Methods:

        // a is an object
        class Foo
          fn say_hi(): puts("Hello world!")

          fn say_hi_twice(): do
            self.say_hi()
            self.say_hi()
          end
        end

        let a = new Foo()
        a.say_hi_twice()

* A type:

        let int_type = int.type()
        int_type == Integer

Defining custom types is simple:

    class MyType
      fn init(x): @x = x
      fn x_is_equal_to(y): @x = y
    end

    let my_obj = new MyType(5)
    let yes = my_obj.x_is_equal_to(5)
    let no = my_obj.x_is_equal_to(47)

* `parent()`&mdash; Returns the parent type of `self`.

#### Iterators and Ranges ####

Iterators are the basic way to, well, iterate over something. A basic iterator
type supports three methods:

* `get()` &mdash; returns the item currently pointed to
* `increment` &mdash; moves the iterator to the next item in its range, and returns
  itself.
* `at_end()` &mdash; returns whether or not the iterator is at the end of its range.
  If it is, the iterator doesn't point to anything valid&mdash; conceptually, it
  works like this:

        { item 0, item 2 ... item n - 1, item n }
          ^                                     ^
          start                                 end

A range is even simpler; it only needs to support one method, `start()`, that
returns an iterator pointing to its first element. In the standard library, both
`Array` and `String` are ranges, and `ArrayIterator` and `String` are their
corresponding iterators. `Range` is both a range (natch!) and an iterator&mdash;
calling `start()` on a range just returns a copy of itself.

Iterators are used to implement for loops:

    for i in range: puts(i)

is equivalent to something like:

    let <implicit_var> = range.start()
    while !<implicit_var>.at_end(): do
      let i = <implicit_var>.get()
      puts(i)
      <implicit_var>.increment()
    end

### Structures ###

Vivaldi expressions comprise:

#### Literals ####
`5`, `5.0`, `true`, `nil`, `"string"`, `'symbol`, `['array]`, and so forth.

#### Variables, Declarations and Assignments ####

All variables must be declared before use:

    i = 5     // wrong: i hasn't been declared yet
    let i = 5 // right
    i = 3     // right: i has been declared, so assigning to it is OK

#### Control Flow ####

Vivaldi's basic control flow structures are cond statements, while loops, and
for loops:

    let i = cond false: "not me!",
                 true:  "me!"
    while true: puts("looping endlessly...")

Cond statements work as in Lisp: the first member of each pair is evaluated
until one is truthy (i.e. not `false` or `nil`). When the truthy member is
found, the second half of that pair is evaluated and returned. If no truthy
tests are found, the statement returns `nil`.

The keyword `if` is provided as a synonym to `cond`, since it reads more
naturally for one-body cond statements:

    // i is a variable
    let a = if i: "i is truthy"
    if a == nil: puts("i isn't truthy")

    // equivalent:
    let a = cond i: "i is truthy"
    cond a == nil: puts("i isn't truthy")

while loops are very straightforward:

    while <condition>: <expression>

For loops are familiar to anyone who's ever used Python or Ruby:

    for <i> in <range>: <expression>

For more on for loops, see iterators.

Note that while and for loops always evaluate to `nil`.

#### Blocks ####

Functions, cond/if statements, and while loops are all limited to a single
expression for their bodies. This kind of sucks if you want to do anything
actually interesting with them. Fortunately, it's possible, using blocks, to
mash a bunch of expressions together in a sequence, and return the last computed
result:

    let i = do
      puts("I'm in a block!")
      5; 4; 3; 2; 1
    end
    i == 1

Using this, it's possible to build actually useful constructs:

    fn filter(array, predicate): do
      let filtered = []
      for i in array: if pred(i): filtered.append(i)
      filtered
    end

Blocks have nested scope:

    do let j = 5 end
    puts(j) // wrong: j is out of scope

    let j = 5
    do puts(j) end // fine

#### Exceptions ####

Like in C++, exceptions in Vivaldi don't have any special type (in fact at the
moment there is no builtin exception type; strings are used instead). Otherwise
they work pretty much as you'd expect:

    let i = try: except 5
    catch e: e + 1
    i == 6

As everywhere else in Vivaldi, the pieces of code following `try` and `catch`
are expressions.

#### Operators ####

Vivaldi operators, aside from `&&`, `||`, `to` (which is syntax sugar for
Range), and `=` (which isn't actually an operator at all&mdash; it just looks like
one), are all just syntax sugar for method calls. Here they are in order of
precedence (basically C precedence, with the bitwise mistake fixed and `**` and
`to` inserted where appropriate):

    a[b]     // a.at(b)
    a[b] = c // a.set_at(b, c)
    !a       // a.not()
    -a       // a.negative()
    ~a       // a.negate()
    a ** b   // a.pow(b)
    a * b    // a.times(b)
    a / b    // a.divides(b)
    a % b    // a.modulo(b)
    a + b    // a.add(b)
    a - b    // a.subtract(b)
    a << b   // a.rshift(b)
    a >> b   // a.lshift(b)
    a & b    // a.bitand(b)
    a ^ b    // a.xor(b)
    a | b    // a.bitor(b)
    a to b   // Range(a, b)
    a < b    // a.less(b)
    a > b    // a.greater(b)
    a <= b   // a.less_equals(b)
    a >= b   // a.greater_equals(b)
    a == b   // a.equals(b)
    a != b   // a.unequal(b)

### Builtins ###

In addition to the above types, Vivaldi has a select few miscellaneous builtins:

#### I/O ####

* `puts(x)`&mdash; as in Ruby, write the passed value plus a newline. Takes only one
  argument.

  To pretty-print your custom types through `puts`, `print`, or on the REPL,
  just define a method `str()`; whatever value that returns will be used as the
  string representation instead of `<object>` (or whatever the class you're
  inheriting from prints out).

* `print(x)`&mdash; identical to `puts`, sans newline.

* `gets()`&mdash; returns a String containing a single line of user input.

* `argv`&mdash; when run from a file, contains an Array of all commend-line
  arguments as Strings. The executable and script names are left off; `argv[0]`
  is the first command-line argument explicitly provided by the user:

        $ cat myfile.vv
        puts(argv[0])
        $ vivaldi ./myfile.vv foo
        foo

#### Functional ####

* `count(x, y)`&mdash; returns the number of members in range `x` such that
  predicate function `y` returns true (or at least truthy).

* `map(x, y)`&mdash; Applies the function `y` to each member of the range `x`,
  and returns an Array containing the results.

* `filter(x, y)`&mdash; Returns an Array containing all the members of range `x`
  such that predicate `y` returns true(thy).

* `reduce(x, y, z)`&mdash; Iteratively applies the binary function `z` to the
  result of the computation (starting with `y`) and each value in the range `x`;
  conceptually it looks something like

        fn reduce(x, y, z): do
          for i in x: y = z(y, i)
          y
        end

* `sort(x)`&mdash; Returns an Array containing the members of range `x` sorted
  using `less`.

* `any(x, y)`&mdash; Returns `true` if any member of range `x` satisfies
  predicate function `y`, and `false` otherwise.

* `all(x, y)`&mdash; Returns `true` if all members of range `x` satisfy
  predicate function `y`, and `false` otherwise.

More to come soon.

#### Other ####

* `quit()`&mdash; Exits the program unconditionally.

* `reverse(x)`&mdash; Reverses the range `x`:

        >>> reverse(1 to 10)
        => [9, 8, 7, 6, 5, 4, 3, 2, 1]
        >>> reverse(['foo, 'bar, 'baz])
        => ['baz, 'bar, 'foo]
        >> reverse("foo")
        => ["o", "o", "f"]

More to be added eventually.

### Example ###

#### FizzBuzz ####

    for i in 1 to 100: cond
      i % 15 == 0: puts("FizzBuzz"),
      i % 5 == 0:  puts("Buzz"),
      i % 3 == 0:  puts("Fizz"),
      true:        puts(i)

See examples folder for more.

### C API ###

The C API is extremely new, largely untested, and probably quite buggy and
incomplete. That said, it should be possible to play around with it some. To
use:

#### Instructions ####

* Include the header [include/vivaldi.h](include/vivaldi.h) in your C code; this
  exposes all the functions and types defined by the C api.

* Define a function `void vv_init_lib(void)`; this will be called when your
  library is loaded by Vivaldi. Define all your custom Vivaldi types, variables,
  and functions here.

* Compile your exension as a shared library with arguments ` -undefined
  dynamic_lookup -shared -fPIC`. The resulting `so` or `dylib` will be available
  to Vivaldi via `require`. For instance, an exension `example.dylib` can be
  accessed via `require "example"`.

#### Tips ####

* Any function beginning `vv_new_` will instantiate a new Vivaldi object with
  the passed value. For instance, `vv_new_int(quant)` defines a new Integer with
  the int value `quant`. On success, they'll return a pointer to the
  instantiated Vivaldi object; on failure, they'll return `vv_null` (which is
  equal to `0` to simplify error checking).

* Like any Vivaldi functions, your C extension functions must return a value
  (namely, a `vv_object_t`). If your function has failed (either because of bad
  input, or because a Vivaldi API function failed), return `vv_null`, and a
  generic exception will be thrown. At the moment, unfortunately, there's no
  support for more fine-grained exception support.

* Standalone Vivaldi functions are defined with a signature of `vv_object_t
  my_C_func(void)`. Vivaldi arguments are accessed via `vv_get_arg(argnum)`. To
  expose the function to Vivaldi, call `vv_new_function(my_C_func,
  expected_number_of_args)` in `vv_init_lib`. Argument number checking is
  handled by Vivaldi.

* Vivaldi types require several components:

  * A name, passed as a `const char*`.

  * A parent class, or `vv_null` to inherit from object.

  * A constructor, which allocates an uninitialized object out of the firmament.
  Typically, your types will include binary blobs of data, since there's only so
  much you can do with builtin types. To create these blobs, call
  `vv_alloc_blob(pointer_to_my_blob, destructor)` (for destructors, see below).
  Constructors will have the signature `vv_object_t my_ctor(void)`. DON'T call
  any `vv_new_` methods in your constructor, or weird side-effects could result.

  * (Assuming your type is created via `vv_alloc_blob`) A destructor, which
  frees any resources held in your binary blob. These functions have the
  signature `void my_dtor(vv_object_t self)`. They're not defined along with
  the type, but rather in your constructor, where they're passed to
  `vv_alloc_blob`.

  * (optionally) an `init` function, which is a Vivaldi method. The methods are
  passed the result of your constructor as `self`, and initialize it to a
  reasonable state.

  * An expected number of arguments for `init`. If `init` is `NULL`, this
  value is ignored.

The function call to create a type is `vv_new_type(name, parent, ctor,
init_or_NULL, init_argc)`.

* Vivaldi methods are defined in one of three ways:

  * For methods taking no arguments (a monop), define a function with the
  signature `vv_object_t my_C_monop(vv_object_t self)`. Register it in
  `vv_init_lib` by calling `vv_add_monop(my_type, method_name, my_C_monop)`.

  * For methods taking one argument (a binop), define a function with the
  signature `vv_object_t my_C_binop(vv_object_t self, vv_object_t arg)`.
  Register it in `vv_init_lib` by calling `vv_add_binop(my_type, method_name,
  my_C_binop)`.

  * For methods taking two or more arguments, define a function with the
  signature `vv_object_t my_C_method(vv_object_t self)`. Like with standalone
  functions, Vivaldi arguments are accessed via `vv_get_arg`. Register it in
  `vv_init_lib` by calling `vv_add_method(my_type, method_name, my_C_method,
  expected_arg_count)`.

Caveat: all `init` functions, even if they take zero or one arguments, can't be
defined as monops or binops! This is because they're special, and are passed as
arguments to `vv_new_type` instead of via `vv_add_method`, because they're
needed to instantiate the Vivalid type.

* To read or write Vivaldi variables, call `vv_let`, `vv_write`, or `vv_read` as
  appropriate.

* All builtin types are exposed in `vivaldi.h` as `vv_builtin_type_NAME`.

* If any objects you instantiate refer to other Vivaldi objects, you have to
  store them as Vivaldi members (via `vv_get_mem` and `vv_set_mem`). If you
  don't, the garbage collector won't know about them, and Bad Things will almost
  certainly happen. Garbage collection for any local VV objects you instantiate
  is handled by the API (aside from `vv_alloc_blob`, which should only be used
  in constructors for this reason).

Other operations are described in `vivaldi.h`. Improved documentation to come!

#### Example ####

Here's the code to a C extension defining a Vivaldi function `x_plus_5` that
takes an argument `x` and returns, well, `x + 5`:

```c
vv_object_t
x_plus_5(void)
{
  vv_object_t arg = vv_get_arg(0);
  if (arg == vv_null)
    return vv_null;

  int x;
  int success = vv_get_int(arg, &x);
  if (success == -1)
    return vv_null;

  return vv_new_int(x + 5);
}

void
vv_init_lib(void)
{
  vv_object_t vivaldi_function = vv_new_function(x_plus_5, 1);
  if (vivaldi_function) {
    vv_symbol_t function_name = vv_make_symbol("x_plus_5");
    vv_let(function_name, vivaldi_function);
  }
}
```

### TODO ###

* Expand the standard library

* Improve C API, particularly as concerns error handling

* Improve performance, especially concerning garbage collection and the call
  stack

* Support standard require paths, so that extensions/files don't have to be
  local (and, by extension, add a standard library)

* Improve commenting

Any and all contributions, questions, bug reports, and angry rants welcome!
