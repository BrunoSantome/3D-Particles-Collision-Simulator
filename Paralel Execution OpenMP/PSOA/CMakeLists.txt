cmake_minimum_required(VERSION 3.20)
project(PSOA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors -O3 -DNDEBUG -fopenmp")
add_executable(PSOA PSOA.cpp)
