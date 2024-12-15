//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef STRING_HPP
#define STRING_HPP

#include <structural/inplace_string.hpp>

#include <array>

namespace parsely
{
// Checks whether a character is one of the blank characters ' ', '\t'
consteval auto is_blank(char const c) -> bool
{
    static constexpr std::array chars = {' ', '\t'};
    return std::ranges::contains(chars, c);
}

// Checks whether a character is one of the whitespace characters ' ', '\t', '\n', '\r', '\v', '\f'
consteval auto is_space(char const c) -> bool
{
    static constexpr std::array chars = {'\n', '\r', '\v', '\f'};
    return is_blank(c) || std::ranges::contains(chars, c);
}

// Checks whether a character is one of the decimal digit characters '0' through '9'
consteval auto is_digit(char const c) -> bool
{
    return c >= '0' && c <= '9';
}

// Checks whether a character is one of the latin alphabetic characters, either uppercase or lowercase
consteval auto is_alpha(char const c) -> bool
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

// Equivalent to `is_alpha(c) || is_digit(c)`
consteval auto is_alnum(char const c) -> bool
{
    return is_alpha(c) || is_digit(c);
}

// Trims excess characters at the front and back of an inplace_string and returns the result
template<structural::inplace_string S, auto Predicate = is_space>
consteval auto trim()
{
    if constexpr (S.empty())
        return S;
    else
    {
        static constexpr auto begin = []
        {
            char const* it = S.begin();
            while (it != S.end() && Predicate(*it))
                ++it;
            return it;
        }();
        static constexpr auto end = []
        {
            char const* it = S.end() - 1;
            while (it != begin && Predicate(*it))
                --it;
            return it + 1;
        }();
        static constexpr auto size = std::distance(begin, end);
        return structural::inplace_string<size>{begin, end};
    }
}

// Splits an inplace_string at a delimiter, returning the result as a tuple of inplace_strings
// If `S` does not contain `Delimiter`, `tuple(S)` is returned.
template<structural::inplace_string S, char Delimiter>
consteval auto split()
{
    static constexpr auto parts_vec = []
    {
        structural::inplace_vector<structural::inplace_string<S.size()>, S.size() + 1> ps;

        char const* begin = S.begin();
        for (auto const* it = begin; it != S.end(); ++it)
        {
            if (*it == Delimiter)
            {
                ps.emplace_back(begin, it);
                begin = it + 1;
            }
        }
        ps.emplace_back(begin, S.end());
        return ps;
    }();

    return []<std::size_t... is>(std::index_sequence<is...>) constexpr
    {
        return std::tuple{structural::inplace_string<parts_vec[is].size()>{parts_vec[is]}...};
    }(std::make_index_sequence<parts_vec.size()>{});
}

// Splits an inplace_string at the first occurrence of a delimiter
// If `S` does not contain `Delimiter`, `tuple(S)` is returned.
// Otherwise, `pair(head, tail)` is returned, where `head` is the part before `Delimiter`, and `tail` the part after.
template<structural::inplace_string S, char Delimiter>
consteval auto split_once()
{
    static constexpr char const* split_point = std::ranges::find(S, Delimiter);
    if constexpr (split_point == S.end())
        return std::tuple(S);
    else
    {
        static constexpr auto first_size  = split_point - S.begin();
        static constexpr auto second_size = S.size() - first_size - 1;
        return std::pair(structural::inplace_string<first_size>(S.begin(), split_point),
                         structural::inplace_string<second_size>(split_point + 1, S.end()));
    }
}

// Splits a production rule of the form "<symbol> : <expression>", where <symbol> is alphanumeric, and the expression
// is not validated. Both symbol and expression have blanks trimmed.
template<structural::inplace_string Production>
consteval auto split_production()
{
    static constexpr auto s = split_once<Production, ':'>();
    static_assert(std::tuple_size_v<decltype(s)> == 2, "Production must have pattern <symbol> : <expression>");

    static constexpr auto symbol     = trim<s.first, is_blank>();
    static constexpr auto expression = trim<s.second, is_blank>();
    static_assert(std::ranges::all_of(symbol, is_alnum), "Symbol must only contain alphanumeric characters");
    static_assert(!symbol.empty(), "Symbol names can't be empty");
    static_assert(!expression.empty(), "Expressions can't be empty");

    return std::pair{symbol, expression};
}

// A production AST node
template<std::size_t N, typename Expression>
struct production
{
    structural::inplace_string<N> symbol;
    Expression                    expression;

    constexpr auto operator==(production const&) const -> bool = default;
};

// An alternatives expression AST node
template<typename... Alternatives>
struct alt_expr
{
    std::tuple<Alternatives...> alternatives;

    constexpr auto operator==(alt_expr const&) const -> bool = default;
};

// A sequence expression AST node
template<typename... Elements>
struct seq_expr
{
    std::tuple<Elements...> sequence;

    constexpr auto operator==(seq_expr const&) const -> bool = default;
};

// A terminal expression AST node
template<std::size_t N>
struct terminal_expr
{
    structural::inplace_string<N> terminal;

    constexpr auto operator==(terminal_expr const&) const -> bool = default;
};

// A nonterminal expression AST node
template<std::size_t N>
struct nonterminal_expr
{
    structural::inplace_string<N> symbol;

    constexpr auto operator==(nonterminal_expr const&) const -> bool = default;
};

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
    return alt_expr<seq_expr<Exprs...>>{std::tuple(expr)};
}

// Helper function to elevate an expression to alt_expr
template<std::size_t N>
consteval auto ensure_alt_expr(terminal_expr<N> expr)
{
    return alt_expr<terminal_expr<N>>{std::tuple(expr)};
}

// Helper function to elevate an expression to alt_expr
template<std::size_t N>
consteval auto ensure_alt_expr(nonterminal_expr<N> expr)
{
    return alt_expr<nonterminal_expr<N>>{std::tuple(expr)};
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
    return seq_expr<terminal_expr<N>>{std::tuple(expr)};
}

// Helper function to elevate an expression to seq_expr
template<std::size_t N>
consteval auto ensure_seq_expr(nonterminal_expr<N> expr)
{
    return seq_expr<nonterminal_expr<N>>{std::tuple(expr)};
}

// Helper function to combine two `alt_expr` parsing results
template<typename... Exprs1, typename... Exprs2>
consteval auto combine(alt_expr<Exprs1...> a, alt_expr<Exprs2...> b) -> alt_expr<Exprs1..., Exprs2...>
{
    return alt_expr<Exprs1..., Exprs2...>{std::tuple_cat(a.alternatives, b.alternatives)};
}

// Helper function to combine two `seq_expr` parsing results
template<typename... Exprs1, typename... Exprs2>
consteval auto combine(seq_expr<Exprs1...> a, seq_expr<Exprs2...> b) -> seq_expr<Exprs1..., Exprs2...>
{
    return seq_expr<Exprs1..., Exprs2...>{std::tuple_cat(a.sequence, b.sequence)};
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
    static constexpr char const* split_point = std::ranges::find_if_not(Expression, is_alnum);
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
                static constexpr auto initial_result  = seq_expr<decltype(prim.first)>{std::tuple(prim.first)};
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
                static constexpr auto initial_result  = alt_expr<decltype(seq.first)>{std::tuple(seq.first)};
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
} // namespace parsely

#endif // STRING_HPP
