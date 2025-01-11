//
// Elvis Parsely
// Copyright (c) 2025 Jan Möller.
//

#include <parsely/utility/grammar_parser.hpp>

#include <catch2/catch_all.hpp>

using namespace parsely::detail;

TEST_CASE("grammar_parser")
{
    constexpr grammar_parser<R"raw(foo: "bar" | "(" bam ")";bam:"|" foo;)raw"> p;

    SECTION("_")
    {
        STATIC_CHECK(p.parse<"_">(""));
        STATIC_CHECK(p.parse<"_">(" "));
        STATIC_CHECK(p.parse<"_">("    "));
        STATIC_CHECK(p.parse<"_">("asd"));
    }
    SECTION("__")
    {
        STATIC_CHECK(!p.parse<"__">(""));
        STATIC_CHECK(!p.parse<"__">("asd"));
        STATIC_CHECK(p.parse<"__">(" "));
        STATIC_CHECK(p.parse<"__">(" asd"));
    }

    SECTION("nonterminal")
    {
        STATIC_CHECK(!p.parse<"nonterminal">(""));
        STATIC_CHECK(p.parse<"nonterminal">("asd"));
        STATIC_CHECK(p.parse<"nonterminal">("foo_bar"));
        STATIC_CHECK(!p.parse<"nonterminal">("%foobar"));
        STATIC_CHECK(!p.parse<"nonterminal">("\"asd\""));
        STATIC_CHECK(p.parse<"nonterminal">("foo bar"));
        STATIC_CHECK(p.parse<"nonterminal">("foo|bar"));
    }

    SECTION("terminal")
    {
        STATIC_CHECK(!p.parse<"terminal">(""));
        STATIC_CHECK(!p.parse<"terminal">("asd"));
        STATIC_CHECK(p.parse<"terminal">("\"asd\""));
        STATIC_CHECK(!p.parse<"terminal">("\"asd"));
        STATIC_CHECK(p.parse<"terminal">("\"asd\" foo|\"bar\""));
    }

    SECTION("prim_expr")
    {
        STATIC_CHECK(!p.parse<"prim_expr">(""));
        STATIC_CHECK(!p.parse<"prim_expr">("\"asd"));
        STATIC_CHECK(p.parse<"prim_expr">("\"asd\""));
        STATIC_CHECK(p.parse<"prim_expr">("\"asd\" foo"));
        STATIC_CHECK(p.parse<"prim_expr">("asd foo"));
    }

    SECTION("seq_expr")
    {
        STATIC_CHECK(!p.parse<"seq_expr">(""));
        STATIC_CHECK(p.parse<"seq_expr">("asd"));
        STATIC_CHECK(p.parse<"seq_expr">("\"asd\""));
        STATIC_CHECK(p.parse<"seq_expr">("\"asd\" foo bar"));
        STATIC_CHECK(p.parse<"seq_expr">("\"asd\" foo | bar"));
    }

    SECTION("alt_expr")
    {
        STATIC_CHECK(!p.parse<"alt_expr">(""));
        STATIC_CHECK(p.parse<"alt_expr">("asd"));
        STATIC_CHECK(p.parse<"alt_expr">("asd|qwe"));
        STATIC_CHECK(p.parse<"alt_expr">("asd|qwe rty | foo"));
    }

    SECTION("expression")
    {
        STATIC_CHECK(p.parse<"expression">("foo"));
    }

    SECTION("production")
    {
        STATIC_CHECK(!p.parse<"production">(""));
        STATIC_CHECK(!p.parse<"production">("asd"));
        STATIC_CHECK(!p.parse<"production">("asd:"));
        STATIC_CHECK(!p.parse<"production">("asd: foo"));
        STATIC_CHECK(p.parse<"production">("asd: foo;"));
        STATIC_CHECK(p.parse<"production">("asd: foo \"bar\" | baz; trailing"));
    }

    SECTION("grammar")
    {
        STATIC_CHECK(!p.parse<"grammar">(""));
        STATIC_CHECK(p.parse<"grammar">("asd: foo;"));
        STATIC_CHECK(p.parse<"grammar">("asd: foo; bar : \"baz\" ;"));
    }
}