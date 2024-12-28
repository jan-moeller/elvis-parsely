//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#include <parsely/utility/parser.hpp>

#include <catch2/catch_all.hpp>

using namespace parsely;

TEST_CASE("parser")
{
    SECTION("terminals")
    {
        using foo_parser = parser<R"(foo: "foo";)">;

        constexpr foo_parser parse;

        SECTION("successful")
        {
            auto result = parse("foo");
            CHECK(result.valid);
            CHECK(result.source_text == "foo");
        }
        SECTION("unsuccessful")
        {
            auto result = parse("bar");
            CHECK(!result.valid);
            CHECK(result.source_text == "");
        }
    }
    SECTION("nonterminals")
    {
        using foo_parser = parser<R"(foo: bar;bar:"bar";)">;

        constexpr foo_parser p;

        SECTION("successful")
        {
            SECTION("first")
            {
                auto result = p.parse<"foo">("bar");
                CHECK(result.valid);
                CHECK(result.source_text == "bar");
                REQUIRE(result.nested);
                CHECK(result->valid);
                CHECK(result->source_text == "bar");
                REQUIRE(result->nested);
                CHECK((*result)->valid);
                CHECK((*result)->source_text == "bar");
                CHECK((*result)->terminal == "bar");
            }

            SECTION("second")
            {
                auto result = p.parse<"bar">("bar");
                CHECK(result.valid);
                CHECK(result.source_text == "bar");
                REQUIRE(result.nested);
                CHECK(result->valid);
                CHECK(result->source_text == "bar");
                CHECK(result->terminal == "bar");
            }
        }

        SECTION("unsuccessful")
        {
            auto result = p.parse<"foo">("bam");
            CHECK(!result.valid);
            CHECK(result.source_text == "");
        }
    }
    SECTION("sequence")
    {
        using foobar_parser = parser<R"(foo: "foo" "bar";)">;

        constexpr foobar_parser parse;

        SECTION("successful")
        {
            auto result = parse("foobar");
            CHECK(result.valid);
            CHECK(result.source_text == "foobar");
            REQUIRE(result.nested);
            CHECK(result->get<0>().valid);
            CHECK(result->get<0>().source_text == "foo");
            CHECK(result->get<0>().terminal == "foo");
            CHECK(result->get<1>().valid);
            CHECK(result->get<1>().source_text == "bar");
            CHECK(result->get<1>().terminal == "bar");
        }

        SECTION("unsuccessful")
        {
            auto result = parse("foobbar");
            CHECK(!result.valid);
            CHECK(result.source_text == "foo");
            REQUIRE(result.nested);
            CHECK(result->get<0>().valid);
            CHECK(result->get<0>().source_text == "foo");
            CHECK(result->get<0>().terminal == "foo");
            CHECK(!result->get<1>().valid);
            CHECK(result->get<1>().source_text == "");
            CHECK(result->get<1>().terminal == "bar");
        }
    }
    SECTION("alternatives")
    {
        using foobar_parser = parser<R"(foo: "foo" | "bar";)">;

        constexpr foobar_parser parse;

        SECTION("successful")
        {
            auto result = parse("bar");
            CHECK(result.valid);
            CHECK(result.source_text == "bar");
            REQUIRE(result.nested);
            CHECK(result->get<1>().valid);
            CHECK(result->get<1>().source_text == "bar");
            CHECK(result->get<1>().terminal == "bar");
        }
        SECTION("unsuccessful")
        {
            auto result = parse("bam");
            CHECK(!result.valid);
            CHECK(result.source_text == "");
            REQUIRE(result.nested);
            CHECK(!result->get<1>().valid);
            CHECK(result->get<1>().source_text == "");
            CHECK(result->get<1>().terminal == "bar");
        }
    }
    SECTION("recursive")
    {
        using foobar_parser = parser<R"raw(foo: "(" bar ")"; bar : foo | "";)raw">;

        constexpr foobar_parser parse;

        SECTION("successful")
        {
            auto result = parse("(())");
            CHECK(result.valid);
            CHECK(result.source_text == "(())");
            REQUIRE(result.nested);
            CHECK(result->get<0>().valid);
            CHECK(result->get<0>().source_text == "(");
            CHECK(result->get<1>().valid);
            CHECK(result->get<1>().source_text == "()");
            CHECK(result->get<2>().valid);
            CHECK(result->get<2>().source_text == ")");

            CHECK(result->get<1>()->get<0>()->get<0>().valid);
            CHECK(result->get<1>()->get<0>()->get<0>().source_text == "(");
            CHECK(result->get<1>()->get<0>()->get<1>().valid);
            CHECK(result->get<1>()->get<0>()->get<1>().source_text == "");
            CHECK(result->get<1>()->get<0>()->get<2>().valid);
            CHECK(result->get<1>()->get<0>()->get<2>().source_text == ")");
        }
        SECTION("unsuccessful")
        {
            auto result = parse("(()");
            CHECK(!result.valid);
            CHECK(result.source_text == "(()");
            REQUIRE(result.nested);
            CHECK(result->get<0>().valid);
            CHECK(result->get<0>().source_text == "(");
            CHECK(result->get<1>().valid);
            CHECK(result->get<1>().source_text == "()");
            CHECK(!result->get<2>().valid);
            CHECK(result->get<2>().source_text == "");

            CHECK(result->get<1>()->get<0>()->get<0>().valid);
            CHECK(result->get<1>()->get<0>()->get<0>().source_text == "(");
            CHECK(result->get<1>()->get<0>()->get<1>().valid);
            CHECK(result->get<1>()->get<0>()->get<1>().source_text == "");
            CHECK(result->get<1>()->get<0>()->get<2>().valid);
            CHECK(result->get<1>()->get<0>()->get<2>().source_text == ")");
        }
    }

    SECTION("simple calculator")
    {
        constexpr structural::inplace_string grammar = R"raw(
            expr: binary_expr | unary_expr;
            binary_expr: unary_expr binop expr;
            unary_expr: unop prim_expr | prim_expr;
            prim_expr: "(" expr ")" | number;
            number: digit number | digit;

            digit: "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9";
            unop: "+" | "-";
            binop: "+" | "-" | "*" | "/";
        )raw";

        constexpr parser<trim<grammar>()> parse;

        CHECK(parse("0"));
        CHECK(parse("1234567890"));
        CHECK(parse("-0"));
        CHECK(parse("+0"));
        CHECK(parse("1+2"));
        CHECK(parse("1*2"));
        CHECK(parse("1-2"));
        CHECK(parse("1/2"));
        CHECK(parse("(1+2)*3"));
        CHECK(parse("-(1+2)*3"));
    }
}