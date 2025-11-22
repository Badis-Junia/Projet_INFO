cmake_minimum_required(VERSION 3.20)
project(SFML_EXAMPLES)
set (CMAKE_CXX_STANDARD 20)

set(SFML_STATIC_LIBRARIES TRUE)

if(MSVC)
    set(SFML_DIR "SFML_3.0.2/lib/cmake/SFML")
endif(MSVC)

set(DEFS "PATH_IMG=\"${CMAKE_CURRENT_SOURCE_DIR}/Pictures/\"")
    add_compile_definitions(${DEFS})

find_package(SFML 3 COMPONENTS Window Graphics System REQUIRED)
link_libraries(SFML::Graphics SFML::Window SFML::System)



add_executable (SFML_Example "test_SFML/main.cpp" "test_SFML/Car.cpp")
add_executable (SFML_Example1 "test_SFML1/main.cpp" "test_SFML1/Follower.cpp" "test_SFML1/Leader.cpp")
add_executable (SFML_Example2 "test_SFML2/main.cpp")
add_executable (SFML_Example3 "test_SFML3/main.cpp" "test_SFML3/Plane.cpp")
add_executable (Projet_INFO "Code/main.cpp" "Code/Sources.hpp")
