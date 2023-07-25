cmake_minimum_required(VERSION 3.20)
project(AOS)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated -Werror -pedantic -O3 -DNDEBUG")
add_executable(AOS AOS.cpp)
