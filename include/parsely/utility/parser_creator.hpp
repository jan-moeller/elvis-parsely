//
// Elvis Parsely
// Copyright (c) 2025 Jan Möller.
//

#ifndef INCLUDE_PARSELY_UTILITY_PARSER_CREATOR_HPP
#define INCLUDE_PARSELY_UTILITY_PARSER_CREATOR_HPP

#include <parsely/utility/grammar_ast.hpp>
#include <parsely/utility/parse_tree_node.hpp>

namespace parsely::detail
{
template<typename Parser, auto Expr>
struct parser_creator;

template<typename Parser, nonterminal_expr Expr>
constexpr auto parse_nonterminal(std::string_view input) -> parse_tree_node<Parser, Expr>
{
    return Parser::template parse<Expr.symbol>(input);
}

template<typename Parser, terminal_expr Expr>
constexpr auto parse_terminal(std::string_view input) -> parse_tree_node<Parser, Expr>
{
    if (input.starts_with(Expr.terminal))
    {
        return parse_tree_node<Parser, Expr>{
            .valid       = true,
            .source_text = input.substr(0, Expr.terminal.size()),
        };
    }
    return parse_tree_node<Parser, Expr>{};
}

template<typename Parser, seq_expr Expr>
constexpr auto parse_seq(std::string_view input) -> parse_tree_node<Parser, Expr>
{
    static constexpr auto sub_parsers = []<std::size_t... is>(std::index_sequence<is...>) constexpr
    {
        return structural::tuple{parser_creator<Parser, structural::get<is>(Expr.sequence)>()()...};
    }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.sequence)>>{});

    static constexpr auto parse_one = []<std::size_t I>(std::string_view& input, bool& valid, std::size_t& consumed)
    {
        if (!valid) // Short-circuit
            return parse_tree_node<Parser, get<I>(Expr.sequence)>{};

        auto r = structural::get<I>(sub_parsers)(input);
        input.remove_prefix(r.source_text.size());
        valid &= r.valid;
        consumed += r.source_text.size();
        return r;
    };
    return [remaining_input = input, &input]<std::size_t... is>(std::index_sequence<is...>) constexpr mutable
    {
        bool        valid    = true;
        std::size_t consumed = 0;
        std::tuple  results { parse_one.template operator()<is>(remaining_input, valid, consumed)... };
        return parse_tree_node<Parser, Expr>{.valid         = valid,
                                             .source_text   = input.substr(0, consumed),
                                             .node_sequence = std::move(results)};
    }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.sequence)>>{});
}

template<typename Parser, alt_expr Expr>
constexpr auto parse_alt(std::string_view input) -> parse_tree_node<Parser, Expr>
{
    static constexpr auto sub_parsers = []<std::size_t... is>(std::index_sequence<is...>) constexpr
    {
        return structural::tuple{parser_creator<Parser, structural::get<is>(Expr.alternatives)>()()...};
    }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.alternatives)>>{});

    constexpr auto create_rettype = []<std::size_t... is>(std::index_sequence<is...>) constexpr
    {
        using T = std::variant<std::invoke_result_t<decltype(get<is>(sub_parsers)), std::string_view>...>;
        return T();
    };
    using return_type = decltype(create_rettype(
        std::make_index_sequence<std::tuple_size_v<decltype(Expr.alternatives)>>{}));

    return [input]<std::size_t... is>(std::index_sequence<is...>)
    {
        return_type result;
        ((result = get<is>(sub_parsers)(input), std::visit([](auto const& r) { return r.valid; }, result)) || ...);

        bool             valid       = std::visit([](auto const& r) { return r.valid; }, result);
        std::string_view source_text = std::visit([](auto const& r) { return r.source_text; }, result);

        return parse_tree_node<Parser, Expr>{.valid             = valid,
                                             .source_text       = source_text,
                                             .node_alternatives = std::move(result)};
    }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.alternatives)>>{});
}

template<typename Parser, rep_expr Expr>
constexpr auto parse_rep(std::string_view input) -> parse_tree_node<Parser, Expr>
{
    static constexpr auto sub_parser = parser_creator<Parser, Expr.element>()();

    auto const  orig_input = input;
    std::size_t consumed   = 0;

    std::vector<parse_tree_node<Parser, Expr.element>> parsed;

    for (auto r = sub_parser(input); r; r = sub_parser(input))
    {
        parsed.push_back(r);
        consumed += r.source_text.size();
        input.remove_prefix(r.source_text.size());
    }

    return parse_tree_node<Parser, Expr>{
        .valid            = true,
        .source_text      = orig_input.substr(0, consumed),
        .node_repetitions = std::move(parsed),
    };
}

template<typename Parser, inbuilt_expr Expr>
constexpr auto parse_inbuilt(std::string_view input) -> parse_tree_node<Parser, Expr>
{
    if constexpr (std::is_invocable_r_v<bool, decltype(Expr.parse), char>)
    {
        if (input.empty() || !Expr.parse(input.front()))
            return parse_tree_node<Parser, Expr>{};
        return parse_tree_node<Parser, Expr>{.valid = true, .source_text = input.substr(0, 1)};
    }
    else if constexpr (std::is_invocable_r_v<std::optional<std::size_t>, decltype(Expr.parse), std::string_view>)
    {
        auto const result = Expr.parse(input);
        if (!result)
            return parse_tree_node<Parser, Expr>{};
        return parse_tree_node<Parser, Expr>{.valid = true, .source_text = input.substr(0, result.value())};
    }
}

// Note: it's important that the parser_creators below don't return a lambda expression since gcc fails to
// constant-evaluate it ("dereferencing null pointer"), possibly due to
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=115878.

#define ELVIS_PARSELY_MAKE_PARSER_CREATOR(EXPR)                                                                        \
    template<typename Parser, detail::EXPR##_expr Expr>                                                                \
    struct parser_creator<Parser, Expr>                                                                                \
    {                                                                                                                  \
        static consteval auto operator()() -> parse_tree_node<Parser, Expr> (*)(std::string_view)                      \
        {                                                                                                              \
            return &parse_##EXPR<Parser, Expr>;                                                                        \
        }                                                                                                              \
    };

ELVIS_PARSELY_MAKE_PARSER_CREATOR(nonterminal)
ELVIS_PARSELY_MAKE_PARSER_CREATOR(terminal)
ELVIS_PARSELY_MAKE_PARSER_CREATOR(seq)
ELVIS_PARSELY_MAKE_PARSER_CREATOR(alt)
ELVIS_PARSELY_MAKE_PARSER_CREATOR(rep)
ELVIS_PARSELY_MAKE_PARSER_CREATOR(inbuilt)

#undef ELVIS_PARSELY_MAKE_PARSER_CREATOR
} // namespace parsely::detail

#endif // INCLUDE_PARSELY_UTILITY_PARSER_CREATOR_HPP
