//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef INCLUDE_PARSELY_UTILITY_GRAMMAR_AST_HPP
#define INCLUDE_PARSELY_UTILITY_GRAMMAR_AST_HPP

#include <parsely/utility/string.hpp>

#include <structural/inplace_string.hpp>
#include <structural/tuple.hpp>

namespace parsely::detail
{
// The grammar root AST node
template<typename... Productions>
struct grammar
{
    structural::tuple<Productions...> productions;

    constexpr auto operator==(grammar const&) const -> bool = default;
};

template<typename... Productions>
consteval auto make_grammar(Productions... productions)
{
    return grammar{structural::tuple{productions...}};
}

// A production AST node
template<std::size_t N, typename Expression>
struct production
{
    structural::inplace_string<N> symbol;
    Expression                    expression;

    constexpr auto operator==(production const&) const -> bool = default;
};

template<std::size_t N, typename Expression>
consteval auto make_production(char const (&symbol)[N], Expression expression)
{
    return production{structural::inplace_string<N>{symbol}, expression};
}

// An alternatives expression AST node
template<typename... Alternatives>
struct alt_expr
{
    structural::tuple<Alternatives...> alternatives;

    constexpr auto operator==(alt_expr const&) const -> bool = default;
};

template<typename... Alternatives>
consteval auto make_alt_expr(Alternatives... alternatives)
{
    return alt_expr{structural::tuple{alternatives...}};
}

// A sequence expression AST node
template<typename... Elements>
struct seq_expr
{
    structural::tuple<Elements...> sequence;

    constexpr auto operator==(seq_expr const&) const -> bool = default;
};

template<typename... Sequence>
consteval auto make_seq_expr(Sequence... sequence)
{
    return seq_expr{structural::tuple{sequence...}};
}

// A repetition expression AST node
template<typename Element>
struct rep_expr
{
    Element element;

    constexpr auto operator==(rep_expr const&) const -> bool = default;
};

template<typename Element>
consteval auto make_rep_expr(Element element)
{
    return rep_expr{element};
}

// A terminal expression AST node
template<std::size_t N>
struct terminal_expr
{
    structural::inplace_string<N> terminal;

    constexpr auto operator==(terminal_expr const&) const -> bool = default;
};

template<std::size_t N>
consteval auto make_terminal_expr(char const (&terminal)[N])
{
    return terminal_expr{structural::inplace_string<N>{terminal}};
}

// A nonterminal expression AST node
template<std::size_t N>
struct nonterminal_expr
{
    structural::inplace_string<N> symbol;

    constexpr auto operator==(nonterminal_expr const&) const -> bool = default;
};

template<std::size_t N>
consteval auto make_nonterminal_expr(char const (&symbol)[N])
{
    return nonterminal_expr{structural::inplace_string<N>{symbol}};
}

// An inbuilt expression AST node
template<std::size_t N, typename Fn>
struct inbuilt_expr
{
    structural::inplace_string<N> name;
    Fn                            parse;

    constexpr auto operator==(inbuilt_expr const&) const -> bool = default;
};

template<std::size_t N, typename Fn>
consteval auto make_inbuilt_expr(char const (&name)[N], Fn&& fn)
{
    return inbuilt_expr{structural::inplace_string<N>{name}, std::forward<Fn>(fn)};
}

inline constexpr auto inbuilt_blank    = make_inbuilt_expr("blank", is_blank);
inline constexpr auto inbuilt_space    = make_inbuilt_expr("space", is_space);
inline constexpr auto inbuilt_digit    = make_inbuilt_expr("digit", is_digit);
inline constexpr auto inbuilt_alpha    = make_inbuilt_expr("alpha", is_alpha);
inline constexpr auto inbuilt_alnum    = make_inbuilt_expr("alnum", is_alnum);
inline constexpr auto inbuilt_nonquote = make_inbuilt_expr("nonquote", [](char const c) { return c != '"'; });
} // namespace parsely::detail

#endif // INCLUDE_PARSELY_UTILITY_GRAMMAR_AST_HPP
