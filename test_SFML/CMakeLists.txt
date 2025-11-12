cmake_minimum_required (VERSION 3.20)
project(SFML_Test)

set(TEST_SRCS "main.cpp" "Car.cpp")

add_executable (SFML_Test ${TEST_SRCS})
