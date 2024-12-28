//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef GRAMMAR_PARSER_HPP
#define GRAMMAR_PARSER_HPP

#include <parsely/utility/grammar_ast.hpp>
#include <parsely/utility/indirect.hpp>
#include <parsely/utility/string.hpp>

#include <structural/inplace_string.hpp>

namespace parsely
{
template<typename, auto>
struct parse_tree_node;

// Parse tree node used for sequence expressions
template<typename Parser, seq_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using nested_type = decltype([]<std::size_t... is>(std::index_sequence<is...>) constexpr mutable
                          {
                              using t = std::tuple<parse_tree_node<Parser, get<is>(Expr.sequence)>...>;
                              return t();
                          }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.sequence)>>{}));

    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text
    nested_type      node_sequence; // Tuple of more parse_tree_nodes

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };

    template<std::size_t I>
    constexpr auto get() -> decltype(auto)
    {
        return std::get<I>(node_sequence);
    }
    template<std::size_t I>
    constexpr auto get() const -> decltype(auto)
    {
        return std::get<I>(node_sequence);
    }
};

// Parse tree node used for alternative expressions
template<typename Parser, alt_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using nested_type = decltype([]<std::size_t... is>(std::index_sequence<is...>) constexpr mutable
                                 {
                                     using t = std::variant<parse_tree_node<Parser, get<is>(Expr.alternatives)>...>;
                                     return t();
                                 }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.alternatives)>>{}));

    bool             valid = false;     // True if parsing successful
    std::string_view source_text;       // Consumed source text
    nested_type      node_alternatives; // Variant containing the matched parse_tree_node

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };

    template<class Self, class Visitor>
    constexpr auto visit(this Self&& self, Visitor&& vis) -> decltype(auto)
    {
        using V = decltype(std::forward_like<Self>(std::declval<nested_type>()));
        return std::visit(std::forward<Visitor>(vis), (V)self.node_alternatives);
    }

    template<class R, class Self, class Visitor>
    constexpr auto visit(this Self&& self, Visitor&& vis) -> R
    {
        using V = decltype(std::forward_like<Self>(std::declval<nested_type>()));
        return std::visit<R>(std::forward<Visitor>(vis), (V)self.node_alternatives);
    }

    template<std::size_t I>
    constexpr auto get() -> decltype(auto)
    {
        return std::get<I>(node_alternatives);
    }
    template<std::size_t I>
    constexpr auto get() const -> decltype(auto)
    {
        return std::get<I>(node_alternatives);
    }
};

// Parse tree node used for terminal expressions
template<typename Parser, terminal_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    static constexpr std::string_view terminal = Expr.terminal;

    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };
};

// Parse tree node used for non-terminal expressions
template<typename Parser, nonterminal_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using nested_type = decltype([]() constexpr
                                 {
                                     static constexpr std::size_t index = []<std::size_t... is>(std::index_sequence<is...>) constexpr
                                     {
                                         std::size_t i = 0;
                                         ((i = is, structural::get<is>(Parser::s_grammar.productions).symbol == Expr.symbol) || ...);
                                         return i;
                                     }(std::make_index_sequence<std::tuple_size_v<decltype(Parser::s_grammar.productions)>>{});

                                     static constexpr auto expression = structural::get<index>(Parser::s_grammar.productions).expression;

                                     using t = indirect<parse_tree_node<Parser, expression>>;
                                     return t();
                                 }());

    static constexpr std::string_view symbol = Expr.symbol;

    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text
    nested_type      nested;        // Indirectly stored result parse_tree_node - may be null iif !valid

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };

    constexpr auto operator*() const& -> decltype(auto) { return nested.operator*(); }
    constexpr auto operator*() const&& -> decltype(auto) { return std::move(nested).operator*(); }
    constexpr auto operator*() & -> decltype(auto) { return nested.operator*(); }
    constexpr auto operator*() && -> decltype(auto) { return std::move(nested).operator*(); }

    constexpr auto operator->() const -> decltype(auto) { return nested.operator->(); }
    constexpr auto operator->() -> decltype(auto) { return nested.operator->(); }
};

namespace detail
{
template<typename Parser, auto Expr>
struct parser_creator;

template<typename Parser, nonterminal_expr Expr>
struct parser_creator<Parser, Expr>
{
    static consteval auto operator()() -> parse_tree_node<Parser, Expr> (*)(std::string_view)
    {
        return [](std::string_view input) -> parse_tree_node<Parser, Expr>
        {
            return Parser::template parse<Expr.symbol>(input);
        };
    }
};
template<typename Parser, terminal_expr Expr>
struct parser_creator<Parser, Expr>
{
    static consteval auto operator()() -> parse_tree_node<Parser, Expr> (*)(std::string_view)
    {
        return [](std::string_view input) -> parse_tree_node<Parser, Expr>
        {
            if (input.starts_with(Expr.terminal))
            {
                return parse_tree_node<Parser, Expr>{
                    .valid       = true,
                    .source_text = input.substr(0, Expr.terminal.size()),
                };
            }
            return parse_tree_node<Parser, Expr>{};
        };
    }
};
template<typename Parser, seq_expr Expr>
struct parser_creator<Parser, Expr>
{
    static consteval auto operator()() -> parse_tree_node<Parser, Expr> (*)(std::string_view)
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
        return [](std::string_view input) -> parse_tree_node<Parser, Expr>
        {
            return [remaining_input = input, &input]<std::size_t... is>(std::index_sequence<is...>) constexpr mutable
            {
                bool        valid    = true;
                std::size_t consumed = 0;
                std::tuple  results { parse_one.template operator()<is>(remaining_input, valid, consumed)... };
                return parse_tree_node<Parser, Expr>{.valid         = valid,
                                                     .source_text   = input.substr(0, consumed),
                                                     .node_sequence = std::move(results)};
            }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.sequence)>>{});
        };
    }
};
template<typename Parser, alt_expr Expr>
struct parser_creator<Parser, Expr>
{
    static consteval auto operator()() -> parse_tree_node<Parser, Expr> (*)(std::string_view)
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

        return [](std::string_view input) -> parse_tree_node<Parser, Expr>
        {
            return [remaining_input = input, &input]<std::size_t... is>(std::index_sequence<is...>) mutable
            {
                return_type result;
                ((result = get<is>(sub_parsers)(input), std::visit([](auto const& r) { return r.valid; }, result))
                 || ...);

                bool             valid       = std::visit([](auto const& r) { return r.valid; }, result);
                std::string_view source_text = std::visit([](auto const& r) { return r.source_text; }, result);

                return parse_tree_node<Parser, Expr>{.valid             = valid,
                                                     .source_text       = source_text,
                                                     .node_alternatives = std::move(result)};
            }(std::make_index_sequence<std::tuple_size_v<decltype(Expr.alternatives)>>{});
        };
    }
};
} // namespace detail

// A parser for the given grammar
template<structural::inplace_string Grammar>
struct parser
{
  private:
    static constexpr auto s_grammar = []
    {
        static constexpr auto g = parse_grammar<Grammar>();
        static_assert(!is_failed_parse(g), "Invalid grammar!");
        static_assert(g.second.empty(), "Excess input at the end of grammar!");
        return g.first;
    }();
    static constexpr std::size_t s_num_productions = std::tuple_size_v<decltype(s_grammar.productions)>;

    template<typename, auto>
    friend struct parse_tree_node;

  public:
    // Parses the given input string and returns a parse tree
    //
    // The Symbol NTTP indicates which production to use for parsing. By default, the production first mentioned in the
    // grammar is used.
    template<structural::inplace_string Symbol = get<0>(s_grammar.productions).symbol>
    static constexpr auto parse(std::string_view const input) -> parse_tree_node<parser, nonterminal_expr{Symbol}>
    {
        static constexpr std::size_t index = []<std::size_t... is>(std::index_sequence<is...>) constexpr
        {
            std::size_t i = 0;
            ((i = is, structural::get<is>(s_grammar.productions).symbol == Symbol) || ...);
            return i;
        }(std::make_index_sequence<s_num_productions>{});
        static_assert(index < s_num_productions, "Unknown symbol!");

        static constexpr auto expression = structural::get<index>(s_grammar.productions).expression;

        static constexpr auto nt_parser = detail::parser_creator<parser, expression>()();

        auto result = nt_parser(input);
        return parse_tree_node<parser, nonterminal_expr{Symbol}>{.valid       = result.valid,
                                                                 .source_text = result.source_text,
                                                                 .nested      = indirect(std::move(result))};
    }

    // Parses the given input string
    static constexpr auto operator()(std::string_view const input) { return parse<>(input); }
};
} // namespace parsely

#endif // GRAMMAR_PARSER_HPP
