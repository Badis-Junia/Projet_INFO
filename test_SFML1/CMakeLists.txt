cmake_minimum_required (VERSION 3.20)
project(SFML_Test1)

set(TEST_SRCS "main.cpp" "Follower.cpp" "Leader.cpp")

add_executable (SFML_Test1 ${TEST_SRCS})

