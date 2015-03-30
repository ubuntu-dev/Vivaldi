cmake_minimum_required (VERSION 3.0)
project (vivaldi)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wold-style-cast -Wconversion -Wsign-conversion -O3 -std=c++1y -stdlib=libc++")
set(CMAKE_EXE_LINKER_FLAGS "-rdynamic -flat_namespace -lc++abi")

# CMake doesn't search /usr/local/* by default, of course, because that would
# indicate some modicum of common sense and God knows we can't have that
include_directories("src" "include" SYSTEM "/usr/local/include")
link_directories("/usr/local/lib")

add_executable(vivaldi
  src/main.cpp

  src/gc.cpp
  src/symbol.cpp
  src/value.cpp
  src/vm.cpp

  src/builtins/object.cpp
  src/builtins/type.cpp

  src/builtins/array.cpp
  src/builtins/bool.cpp
  src/builtins/dictionary.cpp
  src/builtins/file.cpp
  src/builtins/floating_point.cpp
  src/builtins/integer.cpp
  src/builtins/nil.cpp
  src/builtins/range.cpp
  src/builtins/regex.cpp
  src/builtins/string.cpp
  src/builtins/symbol.cpp

  src/builtins.cpp
  src/messages.cpp

  src/tokenizer.cpp
  src/validator.cpp
  src/parser.cpp

  src/expression.cpp
  src/opt.cpp

  src/get_file_contents.cpp
  src/c_api.cpp
  src/repl.cpp

  src/ast/assignment.cpp
  src/ast/array.cpp
  src/ast/block.cpp
  src/ast/cond_statement.cpp
  src/ast/dictionary.cpp
  src/ast/except.cpp
  src/ast/for_loop.cpp
  src/ast/function_call.cpp
  src/ast/function_definition.cpp
  src/ast/literal.cpp
  src/ast/logical_and.cpp
  src/ast/logical_or.cpp
  src/ast/member.cpp
  src/ast/member_assignment.cpp
  src/ast/object_creation.cpp
  src/ast/require.cpp
  src/ast/return_statement.cpp
  src/ast/try_catch.cpp
  src/ast/type_definition.cpp
  src/ast/variable.cpp
  src/ast/variable_declaration.cpp
  src/ast/while_loop.cpp

  src/gc/allocated_block_list.cpp

  src/utils/lang.cpp
  src/utils/memory.cpp
  src/utils/string_helpers.cpp

  src/value/array.cpp
  src/value/array_iterator.cpp
  src/value/basic_function.cpp
  src/value/blob.cpp
  src/value/boolean.cpp
  src/value/builtin_function.cpp
  src/value/dictionary.cpp
  src/value/file.cpp
  src/value/floating_point.cpp
  src/value/function.cpp
  src/value/integer.cpp
  src/value/nil.cpp
  src/value/object.cpp
  src/value/opt_functions.cpp
  src/value/range.cpp
  src/value/regex.cpp
  src/value/string.cpp
  src/value/string_iterator.cpp
  src/value/symbol.cpp
  src/value/type.cpp

  src/vm/call_frame.cpp
  src/vm/instruction.cpp)

target_link_libraries(vivaldi boost_system boost_filesystem)

install(TARGETS vivaldi DESTINATION bin)
install(FILES include/vivaldi.h DESTINATION include)
