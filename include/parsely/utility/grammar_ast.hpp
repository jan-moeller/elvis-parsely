//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef INCLUDE_PARSELY_UTILITY_GRAMMAR_AST_HPP
#define INCLUDE_PARSELY_UTILITY_GRAMMAR_AST_HPP

#include <parsely/utility/string.hpp>

#include <structural/inplace_string.hpp>
#include <structural/tuple.hpp>

#include <algorithm>
#include <tuple>

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

// Helper function to elevate an expression to alt_expr
template<typename... Exprs>
consteval auto ensure_alt_expr(alt_expr<Exprs...> expr)
{
    return expr;
}

// Helper function to elevate an expression to alt_expr
template<typename... Exprs>
consteval auto ensure_alt_expr(seq_expr<Exprs...> expr)
{
    return alt_expr<seq_expr<Exprs...>>{structural::tuple(expr)};
}

// Helper function to elevate an expression to alt_expr
template<std::size_t N>
consteval auto ensure_alt_expr(terminal_expr<N> expr)
{
    return alt_expr<terminal_expr<N>>{structural::tuple(expr)};
}

// Helper function to elevate an expression to alt_expr
template<std::size_t N>
consteval auto ensure_alt_expr(nonterminal_expr<N> expr)
{
    return alt_expr<nonterminal_expr<N>>{structural::tuple(expr)};
}

// Helper function to elevate an expression to seq_expr
template<typename... Exprs>
consteval auto ensure_seq_expr(seq_expr<Exprs...> expr)
{
    return expr;
}

// Helper function to elevate an expression to seq_expr
template<std::size_t N>
consteval auto ensure_seq_expr(terminal_expr<N> expr)
{
    return seq_expr<terminal_expr<N>>{structural::tuple(expr)};
}

// Helper function to elevate an expression to seq_expr
template<std::size_t N>
consteval auto ensure_seq_expr(nonterminal_expr<N> expr)
{
    return seq_expr<nonterminal_expr<N>>{structural::tuple(expr)};
}

// Helper function to combine two `alt_expr` parsing results
template<typename... Exprs1, typename... Exprs2>
consteval auto combine(alt_expr<Exprs1...> a, alt_expr<Exprs2...> b) -> alt_expr<Exprs1..., Exprs2...>
{
    return alt_expr<Exprs1..., Exprs2...>{structural::tuple_cat(a.alternatives, b.alternatives)};
}

// Helper function to combine two `seq_expr` parsing results
template<typename... Exprs1, typename... Exprs2>
consteval auto combine(seq_expr<Exprs1...> a, seq_expr<Exprs2...> b) -> seq_expr<Exprs1..., Exprs2...>
{
    return seq_expr<Exprs1..., Exprs2...>{structural::tuple_cat(a.sequence, b.sequence)};
}

// Helper function to combine two `grammar` parsing results
template<typename... Prods1, typename... Prods2>
consteval auto combine(grammar<Prods1...> a, grammar<Prods2...> b) -> grammar<Prods1..., Prods2...>
{
    return grammar<Prods1..., Prods2...>{structural::tuple_cat(a.productions, b.productions)};
}

// Checks if a parse result is a failure (an empty tuple is used to signal failure)
template<typename T>
consteval auto is_failed_parse(T const& t) -> bool
{
    return std::tuple_size_v<std::remove_cvref_t<decltype(t)>> == 0;
}

// Parses a terminal expression
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(terminal_expr, remaining)`, where `remaining` is the part of `Expression` not consumed, otherwise
template<structural::inplace_string Expression>
consteval auto parse_terminal_expr() // -> pair(terminal_expr<...>, inplace_string)
{
    if constexpr (!Expression.starts_with("\""))
        return std::tuple();
    else
    {
        static constexpr char const* split_point = std::ranges::find(Expression.begin() + 1, Expression.end(), '\"');
        if constexpr (split_point == Expression.end())
            return std::tuple();
        else
        {
            static constexpr std::size_t consumed_size  = split_point - Expression.begin() + 1;
            static constexpr std::size_t remaining_size = Expression.size() - consumed_size;

            static constexpr auto result = terminal_expr<consumed_size - 2>{
                structural::inplace_string<consumed_size - 2>(Expression.begin() + 1, split_point)};

            return std::pair(result, structural::inplace_string<remaining_size>(split_point + 1, Expression.end()));
        }
    }
}

// Parses a nonterminal expression
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(nonterminal_expr, remaining)`, where `remaining` is the part of `Expression` not consumed, otherwise
template<structural::inplace_string Expression>
consteval auto parse_nonterminal_expr() // -> pair(nonterminal_expr<...>, inplace_string)
{
    static constexpr char const* split_point = std::ranges::find_if_not(Expression, is_iden);
    if constexpr (split_point == Expression.begin())
        return std::tuple();
    else
    {
        static constexpr std::size_t consumed_size  = split_point - Expression.begin();
        static constexpr std::size_t remaining_size = Expression.size() - consumed_size;

        static constexpr auto result = nonterminal_expr<consumed_size>{
            structural::inplace_string<consumed_size>(Expression.begin(), split_point)};

        return std::pair(result, structural::inplace_string<remaining_size>(split_point, Expression.end()));
    }
}

// Parses a primary expression
// Returns:
//  `tuple()` in case of parsing failure
//  `pair((non)terminal_expr, remaining)`, where `remaining` is the part of `Expression` not consumed, otherwise
template<structural::inplace_string Expression>
consteval auto parse_prim_expr() // -> pair(terminal_expr<...> | nonterminal_expr<...>, inplace_string)
{
    static constexpr auto terminal = parse_terminal_expr<Expression>();
    if constexpr (is_failed_parse(terminal))
    {
        static constexpr auto nonterminal = parse_nonterminal_expr<Expression>();
        if constexpr (is_failed_parse(nonterminal))
            return std::tuple();
        else
            return nonterminal;
    }
    else
    {
        return terminal;
    }
}

// Parses a sequence expression
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(seq_expr, remaining)`, where `remaining` is the part of `Expression` not consumed, otherwise
template<structural::inplace_string Expression>
consteval auto parse_seq_expr() // -> pair(seq_expr<...>, inplace_string)
{
    static constexpr auto prim = parse_prim_expr<Expression>();
    if constexpr (is_failed_parse(prim))
        return std::tuple();
    else
    {
        static constexpr auto next = trim<prim.second>();
        if constexpr (next == prim.second) // no whitespace follows primary expression
            return prim;
        else
        {
            static constexpr auto result = parse_seq_expr<next>();
            if constexpr (is_failed_parse(result))
                return prim;
            else
            {
                static constexpr auto initial_result  = seq_expr<decltype(prim.first)>{structural::tuple(prim.first)};
                static constexpr auto combined_result = combine(initial_result, ensure_seq_expr(result.first));
                return std::pair(combined_result, result.second);
            }
        }
    }
}

// Parses an alternatives expression
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(alt_expr, remaining)`, where `remaining` is the part of `Expression` not consumed, otherwise
template<structural::inplace_string Expression>
consteval auto parse_alt_expr() // -> pair(alt_expr<...>, inplace_string)
{
    static constexpr auto seq = parse_seq_expr<Expression>();
    if constexpr (is_failed_parse(seq))
        return std::tuple();
    else
    {
        static constexpr auto alt = trim<seq.second>();
        if constexpr (!alt.starts_with("|"))
            return seq;
        else
        {
            static constexpr auto next   = trim<structural::inplace_string<alt.size() - 1>{alt.begin() + 1}>();
            static constexpr auto result = parse_alt_expr<next>();
            if constexpr (is_failed_parse(result))
                return seq;
            else
            {
                static constexpr auto initial_result  = alt_expr<decltype(seq.first)>{structural::tuple(seq.first)};
                static constexpr auto combined_result = combine(initial_result, ensure_alt_expr(result.first));
                return std::pair(combined_result, result.second);
            }
        }
    }
}

// Parses an expression
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(alt_expr, remaining)`, where `remaining` is the part of `Expression` not consumed, otherwise
template<structural::inplace_string Expression>
consteval auto parse_expression() // -> pair(alt_expr<...>, inplace_string)
{
    static constexpr auto alternatives = parse_alt_expr<Expression>();
    if constexpr (is_failed_parse(alternatives))
        return std::tuple();
    else
        return alternatives;
}

// Parses a production
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(production, remaining)`, where `remaining` is the part of `Production` not consumed, otherwise
template<structural::inplace_string Production>
consteval auto parse_production() // -> pair(production<...>, inplace_string)
{
    static constexpr auto symbol = parse_nonterminal_expr<Production>();
    if constexpr (is_failed_parse(symbol))
        return std::tuple();
    else
    {
        static constexpr auto colon_expr = trim<symbol.second>();
        if constexpr (!colon_expr.starts_with(":"))
            return std::tuple();
        else
        {
            static constexpr auto expr = trim<structural::inplace_string<colon_expr.size() - 1>(colon_expr.begin() + 1,
                                                                                                colon_expr.end())>();
            static constexpr auto expression = parse_expression<expr>();
            if constexpr (is_failed_parse(expression))
                return std::tuple();
            else
            {
                static constexpr auto last = trim<expression.second>();
                if constexpr (!last.starts_with(";"))
                    return std::tuple();
                else
                {
                    static constexpr auto remaining = structural::inplace_string<last.size() - 1>(last.begin() + 1,
                                                                                                  last.end());
                    return std::pair{
                        production<symbol.first.symbol.size(), decltype(expression.first)>{
                            symbol.first.symbol,
                            expression.first,
                        },
                        remaining,
                    };
                }
            }
        }
    }
}

// Parses a grammar
// Returns:
//  `tuple()` in case of parsing failure
//  `pair(grammar, remaining)`, where `remaining` is the part of `Grammar` not consumed, otherwise
template<structural::inplace_string Grammar>
consteval auto parse_grammar() // -> pair(grammar<...>, inplace_string)
{
    static constexpr auto prod = parse_production<Grammar>();
    if constexpr (is_failed_parse(prod))
        return std::tuple();
    else
    {
        static constexpr auto remaining      = trim<prod.second>();
        static constexpr auto result         = parse_grammar<remaining>();
        static constexpr auto initial_result = grammar<decltype(prod.first)>{structural::tuple(prod.first)};
        if constexpr (is_failed_parse(result))
            return std::pair(initial_result, prod.second);
        else
        {
            static constexpr auto combined_result = combine(initial_result, result.first);
            return std::pair(combined_result, result.second);
        }
    }
}
} // namespace parsely::detail

#endif // INCLUDE_PARSELY_UTILITY_GRAMMAR_AST_HPP
