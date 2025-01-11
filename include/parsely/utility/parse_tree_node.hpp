//
// Elvis Parsely
// Copyright (c) 2025 Jan Möller.
//

#ifndef INCLUDE_PARSELY_UTILITY_PARSE_TREE_NODE_HPP
#define INCLUDE_PARSELY_UTILITY_PARSE_TREE_NODE_HPP

#include <parsely/utility/grammar_ast.hpp>
#include <parsely/utility/indirect.hpp>

#include <vector>

namespace parsely
{
template<typename, auto>
struct parse_tree_node;

// Parse tree node used for sequence expressions
template<typename Parser, detail::seq_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using parser_type = Parser;
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
    static constexpr std::size_t size = std::tuple_size_v<nested_type>;
};

// Parse tree node used for alternative expressions
template<typename Parser, detail::alt_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using parser_type = Parser;
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

    constexpr auto index() const -> std::size_t
    {
        return node_alternatives.index();
    }
};

// Parse tree node used for repetition expressions
template<typename Parser, detail::rep_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using parser_type = Parser;
    using nested_type = std::vector<parse_tree_node<Parser, Expr.element>>;

    bool             valid = false;    // True if parsing successful
    std::string_view source_text;      // Consumed source text
    nested_type      node_repetitions; // vector of more parse_tree_nodes

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };

    constexpr auto size() const noexcept -> std::size_t { return node_repetitions.size(); }
    constexpr auto empty() const noexcept -> bool { return node_repetitions.empty(); }

    constexpr auto operator[](std::size_t i) const noexcept -> parse_tree_node<Parser, Expr.element> const&
    {
        return node_repetitions[i];
    }
};

// Parse tree node used for terminal expressions
template<typename Parser, detail::terminal_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using parser_type = Parser;
    static constexpr std::string_view terminal = Expr.terminal;

    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };
};

// Parse tree node used for non-terminal expressions
template<typename Parser, detail::nonterminal_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using parser_type = Parser;
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

// Parse tree node used for inbuilt expressions
template<typename Parser, detail::inbuilt_expr Expr>
struct parse_tree_node<Parser, Expr>
{
    using parser_type = Parser;
    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };
};

#define ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(NODE)                                                               \
    template<typename Node>                                                                                            \
    struct is_##NODE##_parse_tree_node : std::false_type                                                               \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template<typename Parser, detail::NODE##_expr Expr>                                                                \
    struct is_##NODE##_parse_tree_node<parse_tree_node<Parser, Expr>> : std::true_type                                 \
    {                                                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    template<typename Node>                                                                                            \
    inline constexpr bool is_##NODE##_parse_tree_node_v = is_##NODE##_parse_tree_node<Node>::value;                    \
                                                                                                                       \
    template<typename Node>                                                                                            \
    concept is_##NODE##_node = is_##NODE##_parse_tree_node_v<Node>;

ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(seq)
ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(alt)
ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(rep)
ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(nonterminal)
ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(terminal)
ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT(inbuilt)

#undef ELVIS_PARSELY_MAKE_PARSE_TREE_NODE_CONCEPT

} // namespace parsely

#endif // INCLUDE_PARSELY_UTILITY_PARSE_TREE_NODE_HPP
