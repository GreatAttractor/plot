#
# Plot
# Copyright (c) 2019 Filip Szczerek <ga.software@yahoo.com>
#
# This project is licensed under the terms of the MIT license
# (see the LICENSE file for details).
#

set(TEST_EXEC plot_test_executable)

add_executable(${TEST_EXEC}
    test/plot_explicit_2d_test.cpp
    include/plot_explicit_2d.hpp
    src/plot_explicit_2d.cpp
)
target_include_directories(${TEST_EXEC} PRIVATE include)

set(WARNINGS -Werror -Wall -Wextra -Wpedantic -Wold-style-cast -Wno-parentheses)
target_compile_options(${TEST_EXEC} PRIVATE ${WARNINGS})
set_property(TARGET ${TEST_EXEC} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${TEST_EXEC} PROPERTY CXX_STANDARD_REQUIRED ON)

include(FindPkgConfig)
find_package(Boost REQUIRED filesystem unit_test_framework)
target_include_directories(${TEST_EXEC} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${TEST_EXEC} ${Boost_LIBRARIES})

add_test(NAME plot COMMAND ${TEST_EXEC})
