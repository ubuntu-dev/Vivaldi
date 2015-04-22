find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${vivaldi_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})

add_executable(test_validator
  validator.cpp)

target_link_libraries(test_validator vivaldi_lib)
