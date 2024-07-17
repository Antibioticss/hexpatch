cmake_minimum_required(VERSION 3.28)
project(hex_patch C)

set(CMAKE_C_STANDARD 11)

add_library(hex_patch STATIC hex_patch.c)

add_executable(tests test.c)

target_link_libraries(tests hex_patch)

enable_testing()
add_test(NAME tests COMMAND time tests)