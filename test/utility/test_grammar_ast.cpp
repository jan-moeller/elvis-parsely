//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#include <parsely/utility/grammar_ast.hpp>

#include <catch2/catch_all.hpp>

using namespace parsely;

TEST_CASE("parse_nonterminal_expr")
{
    STATIC_CHECK(parse_nonterminal_expr<"">() == std::tuple());
    STATIC_CHECK(parse_nonterminal_expr<"asd">() == std::tuple(nonterminal_expr<3>{"asd"}, ""));
    STATIC_CHECK(parse_nonterminal_expr<"\"asd\"">() == std::tuple());
    STATIC_CHECK(parse_nonterminal_expr<"foo bar">() == std::tuple(nonterminal_expr<3>{"foo"}, " bar"));
    STATIC_CHECK(parse_nonterminal_expr<"foo|bar">() == std::tuple(nonterminal_expr<3>{"foo"}, "|bar"));
}

TEST_CASE("parse_terminal_expr")
{
    STATIC_CHECK(parse_terminal_expr<"">() == std::tuple());
    STATIC_CHECK(parse_terminal_expr<"asd">() == std::tuple());
    STATIC_CHECK(parse_terminal_expr<"\"asd\"">() == std::tuple(terminal_expr<3>{"asd"}, ""));
    STATIC_CHECK(parse_terminal_expr<"\"asd">() == std::tuple());
    STATIC_CHECK(parse_terminal_expr<"\"asd\" foo|\"bar\"">() == std::tuple(terminal_expr<3>{"asd"}, " foo|\"bar\""));
}

TEST_CASE("parse_prim_expr")
{
    STATIC_CHECK(parse_prim_expr<"">() == std::tuple());
    STATIC_CHECK(parse_prim_expr<"\"asd">() == std::tuple());
    STATIC_CHECK(parse_prim_expr<"\"asd\"">() == parse_terminal_expr<"\"asd\"">());
    STATIC_CHECK(parse_prim_expr<"\"asd\" foo">() == parse_terminal_expr<"\"asd\" foo">());
    STATIC_CHECK(parse_prim_expr<"asd foo">() == parse_nonterminal_expr<"asd foo">());
}

TEST_CASE("parse_seq_expr")
{
    STATIC_CHECK(parse_seq_expr<"">() == std::tuple());
    STATIC_CHECK(parse_seq_expr<"asd">() == std::tuple(nonterminal_expr<3>{"asd"}, ""));
    STATIC_CHECK(parse_seq_expr<"\"asd\"">() == std::tuple(terminal_expr<3>{"asd"}, ""));
    STATIC_CHECK(parse_seq_expr<"\"asd\" foo bar">()
                 == std::tuple(
                     seq_expr{
                         structural::tuple{
                             terminal_expr<3>{"asd"},
                             nonterminal_expr<3>{"foo"},
                             nonterminal_expr<3>{"bar"},
                         },
                     },
                     ""));
    STATIC_CHECK(parse_seq_expr<"\"asd\" foo | bar">()
                 == std::tuple(
                     seq_expr{
                         structural::tuple{
                             terminal_expr<3>{"asd"},
                             nonterminal_expr<3>{"foo"},
                         },
                     },
                     " | bar"));
}

TEST_CASE("parse_alt_expr")
{
    STATIC_CHECK(parse_alt_expr<"">() == std::tuple());
    STATIC_CHECK(parse_alt_expr<"asd">() == std::tuple(nonterminal_expr<3>{"asd"}, ""));
    STATIC_CHECK(parse_alt_expr<"asd|qwe">()
                 == std::tuple(
                     alt_expr{
                         structural::tuple{
                             nonterminal_expr<3>{"asd"},
                             nonterminal_expr<3>{"qwe"},
                         },
                     },
                     ""));
    STATIC_CHECK(parse_alt_expr<"asd|qwe rty | foo">()
                 == std::tuple(
                     alt_expr{
                         structural::tuple{
                             nonterminal_expr<3>{"asd"},
                             seq_expr{
                                 structural::tuple{
                                     nonterminal_expr<3>{"qwe"},
                                     nonterminal_expr<3>{"rty"},
                                 },
                             },
                             nonterminal_expr<3>{"foo"},
                         },
                     },
                     ""));
}

TEST_CASE("parse_expression")
{
    STATIC_CHECK(parse_expression<"foo">() == parse_alt_expr<"foo">());
}

TEST_CASE("parse_production")
{
    STATIC_CHECK(parse_production<"">() == std::tuple());
    STATIC_CHECK(parse_production<"asd">() == std::tuple());
    STATIC_CHECK(parse_production<"asd:">() == std::tuple());
    STATIC_CHECK(parse_production<"asd: foo">() == std::tuple());
    STATIC_CHECK(parse_production<"asd: foo;">()
                 == std::tuple(
                     production<3, nonterminal_expr<3>>{
                         "asd",
                         nonterminal_expr<3>{"foo"},
                     },
                     ""));
    STATIC_CHECK(parse_production<"asd: foo \"bar\" | baz; trailing">()
                 == std::tuple(
                     production<3, alt_expr<seq_expr<nonterminal_expr<3>, terminal_expr<3>>, nonterminal_expr<3>>>{
                         "asd",
                         alt_expr{
                             structural::tuple{
                                 seq_expr{
                                     structural::tuple{
                                         nonterminal_expr<3>{"foo"},
                                         terminal_expr<3>{"bar"},
                                     },
                                 },
                                 nonterminal_expr<3>{"baz"},
                             },
                         },
                     },
                     " trailing"));
}

TEST_CASE("parse_grammar")
{
    STATIC_CHECK(parse_grammar<"">() == std::tuple());
    STATIC_CHECK(parse_grammar<"asd: foo;">()
                 == std::tuple(
                     grammar<production<3, nonterminal_expr<3>>>{
                         structural::tuple{
                             production<3, nonterminal_expr<3>>{"asd", "foo"},
                         },
                     },
                     ""));
    STATIC_CHECK(parse_grammar<"asd: foo; bar : \"baz\" ;">()
                 == std::tuple(
                     grammar<production<3, nonterminal_expr<3>>, production<3, terminal_expr<3>>>{
                         structural::tuple{
                             production<3, nonterminal_expr<3>>{"asd", "foo"},
                             production<3, terminal_expr<3>>{"bar", "baz"},
                         },
                     },
                     ""));
}