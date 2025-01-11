#
# Elvis Parsely
# Copyright (c) 2024 Jan Möller.
#

cmake_minimum_required(VERSION 3.30 FATAL_ERROR)
project(elvis_parsely VERSION 0.0.0)

include(cmake/CPM.cmake)

CPMAddPackage("gh:jan-moeller/structural@0.4.0")

add_library(elvis_parsely INTERFACE
        include/parsely/parsely.hpp
        include/parsely/utility/grammar_ast.hpp
        include/parsely/utility/indirect.hpp
        include/parsely/utility/parse_tree_node.hpp
        include/parsely/utility/parser.hpp
        include/parsely/utility/string.hpp
)
target_include_directories(elvis_parsely INTERFACE include)
target_link_libraries(elvis_parsely INTERFACE structural)

option(ELVIS_PARSELY_ENABLE_TESTING OFF)
set(ELIVS_PARSELY_SANITIZE_TESTS "" CACHE STRING "The sanitizers to enable")

if (ELVIS_PARSELY_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif ()
