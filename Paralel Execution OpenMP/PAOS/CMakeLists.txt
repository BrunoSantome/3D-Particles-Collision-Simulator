cmake_minimum_required(VERSION 3.20)
project(PAOS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors -DNDEBUG -O3 -fopenmp")
add_executable(PAOS PAOS.cpp)
