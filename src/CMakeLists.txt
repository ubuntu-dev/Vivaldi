find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(
  ${vivaldi_SOURCE_DIR}/src
  ${vivaldi_SOURCE_DIR}/include
  SYSTEM ${Boost_INCLUDE_DIRS})

add_library(vivaldi_lib
  ${vivaldi_SOURCE_DIR}/src/gc.cpp
  ${vivaldi_SOURCE_DIR}/src/symbol.cpp
  ${vivaldi_SOURCE_DIR}/src/value.cpp
  ${vivaldi_SOURCE_DIR}/src/vm.cpp

  ${vivaldi_SOURCE_DIR}/src/builtins/object.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/type.cpp

  ${vivaldi_SOURCE_DIR}/src/builtins/array.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/character.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/dictionary.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/exception.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/file.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/floating_point.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/function.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/integer.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/range.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/regex.cpp
  ${vivaldi_SOURCE_DIR}/src/builtins/string.cpp

  ${vivaldi_SOURCE_DIR}/src/builtins.cpp
  ${vivaldi_SOURCE_DIR}/src/messages.cpp

  ${vivaldi_SOURCE_DIR}/src/tokenizer.cpp
  ${vivaldi_SOURCE_DIR}/src/validator.cpp
  ${vivaldi_SOURCE_DIR}/src/parser.cpp

  ${vivaldi_SOURCE_DIR}/src/expression.cpp
  ${vivaldi_SOURCE_DIR}/src/opt.cpp

  ${vivaldi_SOURCE_DIR}/src/get_file_contents.cpp
  ${vivaldi_SOURCE_DIR}/src/c_api.cpp
  ${vivaldi_SOURCE_DIR}/src/repl.cpp

  ${vivaldi_SOURCE_DIR}/src/ast/assignment.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/array.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/block.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/cond_statement.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/dictionary.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/except.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/for_loop.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/function_call.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/function_definition.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/literal.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/logical_and.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/logical_or.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/member.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/member_assignment.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/method.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/object_creation.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/require.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/return_statement.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/try_catch.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/type_definition.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/variable.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/variable_declaration.cpp
  ${vivaldi_SOURCE_DIR}/src/ast/while_loop.cpp

  ${vivaldi_SOURCE_DIR}/src/gc/managed_ptr.cpp
  ${vivaldi_SOURCE_DIR}/src/gc/block_list.cpp

  ${vivaldi_SOURCE_DIR}/src/utils/lang.cpp
  ${vivaldi_SOURCE_DIR}/src/utils/string_helpers.cpp

  ${vivaldi_SOURCE_DIR}/src/value/array.cpp
  ${vivaldi_SOURCE_DIR}/src/value/array_iterator.cpp
  ${vivaldi_SOURCE_DIR}/src/value/basic_object.cpp
  ${vivaldi_SOURCE_DIR}/src/value/blob.cpp
  ${vivaldi_SOURCE_DIR}/src/value/builtin_function.cpp
  ${vivaldi_SOURCE_DIR}/src/value/dictionary.cpp
  ${vivaldi_SOURCE_DIR}/src/value/exception.cpp
  ${vivaldi_SOURCE_DIR}/src/value/file.cpp
  ${vivaldi_SOURCE_DIR}/src/value/floating_point.cpp
  ${vivaldi_SOURCE_DIR}/src/value/function.cpp
  ${vivaldi_SOURCE_DIR}/src/value/method.cpp
  ${vivaldi_SOURCE_DIR}/src/value/object.cpp
  ${vivaldi_SOURCE_DIR}/src/value/opt_functions.cpp
  ${vivaldi_SOURCE_DIR}/src/value/partial_function.cpp
  ${vivaldi_SOURCE_DIR}/src/value/range.cpp
  ${vivaldi_SOURCE_DIR}/src/value/regex.cpp
  ${vivaldi_SOURCE_DIR}/src/value/string.cpp
  ${vivaldi_SOURCE_DIR}/src/value/string_iterator.cpp
  ${vivaldi_SOURCE_DIR}/src/value/type.cpp

  ${vivaldi_SOURCE_DIR}/src/vm/call_frame.cpp
  ${vivaldi_SOURCE_DIR}/src/vm/instruction.cpp)

target_link_libraries(vivaldi_lib
  ${Boost_FILESYSTEM_LIBRARY}
  ${boost_system_library})
