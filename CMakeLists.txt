cmake_minimum_required (VERSION 3.0)
project (vivaldi)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wold-style-cast -Wconversion -Wsign-conversion -O3 -std=c++1y -stdlib=libc++ -ggdb")
set(CMAKE_EXE_LINKER_FLAGS "-rdynamic -flat_namespace -lc++abi")

add_subdirectory(src)
add_subdirectory(test)

include_directories(${vivaldi_SOURCE_DIR}/src)

add_executable(vivaldi
  ${vivaldi_SOURCE_DIR}/src/main.cpp)

target_link_libraries(vivaldi vivaldi_lib)

enable_testing()
add_test(NAME validator COMMAND test_validator)
add_test(NAME values COMMAND test_values)
add_test(NAME string_helpers COMMAND test_string_helpers)

install(FILES ${vivaldi_SOURCE_DIR}/include/vivaldi.h DESTINATION include)
install(TARGETS vivaldi DESTINATION bin)
