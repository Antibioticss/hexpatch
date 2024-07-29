cmake_minimum_required(VERSION 3.28)
project(hexpatch C)

set(CMAKE_C_STANDARD 11)

add_library(hexpatch STATIC hexpatch.c)

add_executable(tests test.c)

target_link_libraries(tests hexpatch)

add_compile_options(-fsanitize=address -Wall)

enable_testing()
add_test(NAME tests COMMAND time tests)