cmake_minimum_required(VERSION 3.10)
project(Labs C)

set(CMAKE_C_STANDARD 11)
add_executable(threads_with_conditions threads_with_conditions.c)

target_link_libraries(threads_with_conditions pthread)

add_executable(prodcons1 prodcons1.c)
target_link_libraries(prodcons1 pthread)
