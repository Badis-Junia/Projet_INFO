cmake_minimum_required (VERSION 3.20)
project(SFML_Test3)

set(TEST_SRCS "main.cpp" "Plane.cpp")

find_package (Threads)

add_executable(SFML_Test3 ${TEST_SRCS})
target_link_libraries (SFML_Test3 Threads::Threads)
