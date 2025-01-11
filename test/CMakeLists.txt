#
# Elvis Parsely
# Copyright (c) 2024 Jan Möller.
#

CPMAddPackage("gh:catchorg/Catch2@3.7.0")

list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(elvis_parsely_tests
        utility/test_grammar_parser.cpp
        utility/test_indirect.cpp
        utility/test_parser_creator.cpp
        utility/test_parser.cpp
        utility/test_string.cpp
)

target_link_libraries(elvis_parsely_tests Catch2::Catch2WithMain elvis_parsely)
set_target_properties(elvis_parsely_tests PROPERTIES
        CXX_STANDARD 26
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

if (ELIVS_PARSELY_SANITIZE_TESTS)
    target_compile_options(elvis_parsely_tests PRIVATE "$<$<CXX_COMPILER_ID:GNU>:-fsanitize=${ELIVS_PARSELY_SANITIZE_TESTS}>")
    target_link_options(elvis_parsely_tests PRIVATE "$<$<CXX_COMPILER_ID:GNU>:-fsanitize=${ELIVS_PARSELY_SANITIZE_TESTS}>")
endif ()

catch_discover_tests(elvis_parsely_tests)
