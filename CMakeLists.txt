#
# Plot
# Copyright (c) 2019 Filip Szczerek <ga.software@yahoo.com>
#
# This project is licensed under the terms of the MIT license
# (see the LICENSE file for details).
#

project(libplot)

cmake_minimum_required(VERSION 3.1)

add_library(plot STATIC
    src/plot_explicit_2d.cpp
)

target_include_directories(plot
    #PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE include
    PRIVATE src
)

set(WARNINGS -Werror -Wall -Wextra -Wpedantic -Wold-style-cast -Wno-parentheses)

target_compile_options(plot PRIVATE ${WARNINGS})

set_property(TARGET plot PROPERTY CXX_STANDARD 17)
set_property(TARGET plot PROPERTY CXX_STANDARD_REQUIRED ON)

enable_testing()
include(test/CMakeLists.txt)
