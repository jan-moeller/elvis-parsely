//
// Elvis Parsely
// Copyright (c) 2025 Jan Möller.
//

#include <parsely/utility/parser_creator.hpp>

#include <catch2/catch_all.hpp>

using namespace parsely::detail;

TEST_CASE("parser_creator")
{
    SECTION("alt_expr")
    {
        SECTION("One alternative")
        {
            constexpr parser_creator<int, make_alt_expr(make_terminal_expr("a"))> creator;
            constexpr auto                                                        parse = creator();

            STATIC_CHECK(!parse(""));
            STATIC_CHECK(!parse("b"));
            STATIC_CHECK(parse("a"));
            STATIC_CHECK(parse("abc"));
        }
        SECTION("Two alternatives")
        {
            constexpr parser_creator<int, make_alt_expr(make_terminal_expr("a"), make_terminal_expr("b"))> creator;
            constexpr auto parse = creator();

            STATIC_CHECK(!parse(""));
            STATIC_CHECK(!parse("c"));
            STATIC_CHECK(parse("a"));
            STATIC_CHECK(parse("b"));
            STATIC_CHECK(parse("abc"));
        }
    }

    SECTION("seq_expr")
    {
        SECTION("Sequence of one")
        {
            constexpr parser_creator<int, make_seq_expr(make_terminal_expr("a"))> creator;
            constexpr auto                                                        parse = creator();

            STATIC_CHECK(!parse(""));
            STATIC_CHECK(!parse("b"));
            STATIC_CHECK(parse("a"));
            STATIC_CHECK(parse("abc"));
        }
        SECTION("Sequence of two")
        {
            constexpr parser_creator<int, make_seq_expr(make_terminal_expr("a"), make_terminal_expr("b"))> creator;
            constexpr auto parse = creator();

            STATIC_CHECK(!parse(""));
            STATIC_CHECK(!parse("b"));
            STATIC_CHECK(!parse("a"));
            STATIC_CHECK(parse("ab"));
            STATIC_CHECK(parse("abc"));
        }
    }

    SECTION("rep_expr")
    {
        constexpr parser_creator<int, make_rep_expr(make_terminal_expr("a"))> creator;
        constexpr auto                                                        parse = creator();

        STATIC_CHECK(parse("").empty());
        STATIC_CHECK(parse("aaa").size() == 3);
        STATIC_CHECK(parse("aaabb").size() == 3);
    }
}