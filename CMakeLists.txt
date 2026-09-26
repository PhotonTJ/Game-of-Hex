cmake_minimum_required(VERSION 3.15)
project(game_of_hex CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
# hex_engine_core is a static library that also gets linked into the
# pybind11 shared module, so it must be position-independent either way.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(HEX_BUILD_TESTS "Build the doctest unit test binary" ON)
option(HEX_BUILD_TOOLS "Build the AI benchmark CLI tool" ON)
option(HEX_BUILD_PYTHON_BINDINGS "Build the pybind11 Python extension module" OFF)

add_subdirectory(engine)
