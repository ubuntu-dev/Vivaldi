find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${vivaldi_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})

add_executable(test_validator
  ${vivaldi_SOURCE_DIR}/src/tokenizer.cpp
  ${vivaldi_SOURCE_DIR}/src/validator.cpp
  ${vivaldi_SOURCE_DIR}/src/utils/string_helpers.cpp
  validator.cpp)
