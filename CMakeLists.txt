cmake_minimum_required(VERSION 3.0.0)
project(hash-table VERSION 0.1.0)

include(CTest)
enable_testing()
add_subdirectory(laba_)

add_executable(hash-table main_memv.c)
add_executable(hash-table-filev map_filev.c)
add_executable(hash-test test_table.cpp main_memv.c)
target_compile_definitions(hash-test PUBLIC TEST)
target_link_libraries(hash-test Catch2::Catch2)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
