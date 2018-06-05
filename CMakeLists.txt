cmake_minimum_required(VERSION 3.10)
project(Labs C)

set(CMAKE_C_STANDARD 11)
add_executable(threads_with_conditions threads_with_conditions.c)

target_link_libraries(threads_with_conditions pthread)

add_executable(prodcons3 p3140076-p3140092-p3140219-prodcons3.c)
add_executable(prodcons2 p3140076-p3140092-p3140219-prodcons2.c)
add_executable(prodcons1 p3140076-p3140092-p3140219-prodcons1.c)

target_link_libraries(prodcons1 pthread)
target_link_libraries(prodcons2 pthread)
target_link_libraries(prodcons3 pthread)
