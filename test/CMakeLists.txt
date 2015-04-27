find_package(Boost COMPONENTS unit_test_framework REQUIRED)
include_directories(${vivaldi_SOURCE_DIR}/src ${Boost_INCLUDE_DIRS})

add_executable(test_string_helpers string_helpers.cpp)
add_executable(test_validator      validator.cpp)
add_executable(test_values         values.cpp)
add_executable(test_vm_instrs      vm_instrs.cpp)

target_link_libraries(test_string_helpers vivaldi_lib)
target_link_libraries(test_validator      vivaldi_lib)
target_link_libraries(test_values         vivaldi_lib)
target_link_libraries(test_vm_instrs      vivaldi_lib)
