cmake_minimum_required(VERSION 3.20)
project(hexpatch C)

add_compile_options(-fsanitize=address,undefined -Wall)
add_link_options(-fsanitize=address,undefined -Wall)

set(CMAKE_C_STANDARD 11)

add_library(hexpatch STATIC hexpatch.c)

add_executable(tests test.c)

target_link_libraries(tests hexpatch)

enable_testing()
add_test(NAME tests COMMAND tests)