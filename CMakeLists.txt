cmake_minimum_required(VERSION 2.8)

# [ Project Settings ]
## Common
project(libngkh)
set(CMAKE_C_FLAGS   "-Wall")
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_C_STANDARD   99)
set(CMAKE_CXX_STANDARD 98)

# [ For buiding this projects ]
## Libraries
include_directories("include")
add_library(ngkh_subproc subproc/subproc.c)

