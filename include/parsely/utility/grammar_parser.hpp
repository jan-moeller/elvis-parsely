//
// Elvis Parsely
// Copyright (c) 2025 Jan Möller.
//

#ifndef INCLUDE_PARSELY_UTILITY_GRAMMAR_PARSER_HPP
#define INCLUDE_PARSELY_UTILITY_GRAMMAR_PARSER_HPP

#include <parsely/utility/grammar_ast.hpp>
#include <parsely/utility/parser_creator.hpp>

#include <structural/inplace_string.hpp>
#include <structural/structuralize.hpp>
#include <structural/tuple.hpp>

namespace parsely::detail
{
template<structural::inplace_string GrammarDescription>
struct grammar_parser
{
    static constexpr structural::inplace_string s_grammar_description = GrammarDescription;

    // grammar     : _ (production _)+ eoi
    // production  : nonterminal _ ":" _ expression _ ";"
    // expression  : alt_expr
    // alt_expr    : seq_expr (_ "|" _ seq_expr)*
    // seq_expr    : prim_expr (__ prim_expr)*
    // prim_expr   : paren_expr | terminal | nonterminal
    // paren_expr  : "(" _ expression _ ")"
    // terminal    : "\"" .* "\""
    // nonterminal : (alnum | "_")+
    // __          : space+
    // _           : __?
    static constexpr auto s_grammar = make_grammar(
        // grammar: _ (production _)+ $eoi;
        make_production("grammar",
                        make_seq_expr( //
                            make_nonterminal_expr("_"),
                            make_nonterminal_expr("production"),
                            make_rep_expr(                                //
                                make_seq_expr(make_nonterminal_expr("_"), //
                                              make_nonterminal_expr("production"))),
                            make_nonterminal_expr("_"),
                            inbuilt_eoi)),
        // production: nonterminal _ ":" _ expression _ ";" ;
        make_production("production",
                        make_seq_expr( //
                            make_nonterminal_expr("nonterminal"),
                            make_nonterminal_expr("_"),
                            make_terminal_expr(":"),
                            make_nonterminal_expr("_"),
                            make_nonterminal_expr("expression"),
                            make_nonterminal_expr("_"),
                            make_terminal_expr(";"))),
        // expression: alt_expr ;
        make_production("expression", make_nonterminal_expr("alt_expr")),
        // alt_expr: seq_expr ( _ "|" _ seq_expr )* ;
        make_production("alt_expr",
                        make_seq_expr( //
                            make_nonterminal_expr("seq_expr"),
                            make_rep_expr(     //
                                make_seq_expr( //
                                    make_nonterminal_expr("_"),
                                    make_terminal_expr("|"),
                                    make_nonterminal_expr("_"),
                                    make_nonterminal_expr("seq_expr"))))),
        // seq_expr: prim_expr ( __ prim_expr )* ;
        make_production("seq_expr",
                        make_seq_expr( //
                            make_nonterminal_expr("prim_expr"),
                            make_rep_expr(     //
                                make_seq_expr( //
                                    make_nonterminal_expr("__"),
                                    make_nonterminal_expr("prim_expr"))))),
        // prim_expr: terminal | nonterminal ;
        make_production("prim_expr",
                        make_alt_expr( //
                            make_nonterminal_expr("paren_expr"),
                            make_nonterminal_expr("terminal"),
                            make_nonterminal_expr("nonterminal"))),
        // paren_expr  : "(" _ expression _ ")" ;
        make_production("paren_expr",
                        make_seq_expr( //
                            make_terminal_expr("("),
                            make_nonterminal_expr("expression"),
                            make_terminal_expr(")"))),
        // terminal: "\"" literal "\"" ;
        make_production("terminal",
                        make_seq_expr( //
                            make_terminal_expr("\""),
                            make_nonterminal_expr("literal"),
                            make_terminal_expr("\""))),
        // literal: $not_quote* ;
        make_production("literal", make_rep_expr(inbuilt_nonquote)),
        // nonterminal: id_char id_char* ;
        make_production("nonterminal",
                        make_seq_expr( //
                            make_nonterminal_expr("id_char"),
                            make_rep_expr(make_nonterminal_expr("id_char")))),
        // id_char: $alnum | "_" ;
        make_production("id_char",
                        make_alt_expr( //
                            inbuilt_alnum,
                            make_terminal_expr("_"))),
        // __: $space _ ;
        make_production("__",
                        make_seq_expr( //
                            inbuilt_space,
                            make_nonterminal_expr("_"))),
        // _: $space* ;
        make_production("_", make_rep_expr(inbuilt_space)));
    static constexpr std::size_t s_num_productions = std::tuple_size_v<decltype(s_grammar.productions)>;

    static constexpr auto get_source_text_range(std::string_view source_text) -> std::array<std::size_t, 2>
    {
        std::size_t const begin = source_text.data() - s_grammar_description.data();
        return {begin, source_text.size()};
    }
    static constexpr auto get_source_text(std::array<std::size_t, 2> source_text_range) -> std::string_view
    {
        return std::string_view{s_grammar_description}.substr(source_text_range[0], source_text_range[1]);
    }

    template<structural::inplace_string Symbol = "grammar">
    static constexpr auto parse(std::string_view const input = s_grammar_description)
        -> parse_tree_node<grammar_parser, nonterminal_expr{Symbol}>
    {
        return detail::parse_nonterminal<grammar_parser, nonterminal_expr{Symbol}>(input);
    }
};
} // namespace parsely::detail

namespace structural
{
template<typename T>
struct serializer<parsely::indirect<T>>
{
    static constexpr void do_serialize(parsely::indirect<T> const&           value,
                                       std::output_iterator<std::byte> auto& out_iter)
    {
        serialize(value == nullptr, out_iter);
        if (value != nullptr)
            serialize(*value, out_iter);
    }

    static constexpr auto do_deserialize(std::in_place_type_t<parsely::indirect<T>>, std::input_iterator auto& in_iter)
        -> parsely::indirect<T>
    {
        bool const is_nullptr = deserialize(std::in_place_type<bool>, in_iter);
        if (is_nullptr)
            return parsely::indirect<T>(nullptr);
        return parsely::indirect<T>(deserialize(std::in_place_type<T>, in_iter));
    }
};

template<inplace_string GrammarDescription, auto Expr>
using grammar_parse_tree_node = parsely::parse_tree_node<parsely::detail::grammar_parser<GrammarDescription>, Expr>;

template<inplace_string GrammarDescription, auto Expr>
struct serializer<grammar_parse_tree_node<GrammarDescription, Expr>>
{
    using node              = grammar_parse_tree_node<GrammarDescription, Expr>;
    using parser            = typename node::parser_type;
    using source_text_range = std::array<std::size_t, 2>;

    static constexpr void do_serialize(node const& value, std::output_iterator<std::byte> auto& out_iter)
    {
        serialize(value.valid, out_iter);
        serialize(parser::get_source_text_range(value.source_text), out_iter);
        if constexpr (requires { value.nested; })
            serialize(value.nested, out_iter);
        else if constexpr (requires { value.node_sequence; })
            serialize(value.node_sequence, out_iter);
        else if constexpr (requires { value.node_alternatives; })
            serialize(value.node_alternatives, out_iter);
        else if constexpr (requires { value.node_repetitions; })
            serialize(value.node_repetitions, out_iter);
    }

    static constexpr auto do_deserialize(std::in_place_type_t<node>, std::input_iterator auto& in_iter) -> node
    {
        if constexpr (requires(node n) { n.nested; })
        {
            return node{
                .valid       = deserialize(std::in_place_type<bool>, in_iter),
                .source_text = parser::get_source_text(deserialize(std::in_place_type<source_text_range>, in_iter)),
                .nested      = deserialize(std::in_place_type<typename node::nested_type>, in_iter),
            };
        }
        else if constexpr (requires(node n) { n.node_sequence; })
        {
            return node{
                .valid         = deserialize(std::in_place_type<bool>, in_iter),
                .source_text   = parser::get_source_text(deserialize(std::in_place_type<source_text_range>, in_iter)),
                .node_sequence = deserialize(std::in_place_type<typename node::nested_type>, in_iter),
            };
        }
        else if constexpr (requires(node n) { n.node_alternatives; })
        {
            return node{
                .valid       = deserialize(std::in_place_type<bool>, in_iter),
                .source_text = parser::get_source_text(deserialize(std::in_place_type<source_text_range>, in_iter)),
                .node_alternatives = deserialize(std::in_place_type<typename node::nested_type>, in_iter),
            };
        }
        else if constexpr (requires(node n) { n.node_repetitions; })
        {
            return node{
                .valid       = deserialize(std::in_place_type<bool>, in_iter),
                .source_text = parser::get_source_text(deserialize(std::in_place_type<source_text_range>, in_iter)),
                .node_repetitions = deserialize(std::in_place_type<typename node::nested_type>, in_iter),
            };
        }
        else
        {
            return node{
                .valid       = deserialize(std::in_place_type<bool>, in_iter),
                .source_text = parser::get_source_text(deserialize(std::in_place_type<source_text_range>, in_iter)),
            };
        }
    }
};

#define STRUCTURAL_MAKE_NODE(name)                                                                                     \
    template<inplace_string GrammarDescription>                                                                        \
    using grammar_parse_tree_node_##name = grammar_parse_tree_node<GrammarDescription,                                 \
                                                                   parsely::detail::make_nonterminal_expr(#name)>;     \
    template<inplace_string GrammarDescription, wrapper WrappedValue>                                                  \
    struct structuralizer<grammar_parse_tree_node_##name<GrammarDescription>, WrappedValue>;

STRUCTURAL_MAKE_NODE(grammar)
STRUCTURAL_MAKE_NODE(production)
STRUCTURAL_MAKE_NODE(expression)
STRUCTURAL_MAKE_NODE(alt_expr)
STRUCTURAL_MAKE_NODE(seq_expr)
STRUCTURAL_MAKE_NODE(prim_expr)
STRUCTURAL_MAKE_NODE(paren_expr)
STRUCTURAL_MAKE_NODE(terminal)
STRUCTURAL_MAKE_NODE(nonterminal)

#undef STRUCTURAL_MAKE_NODE

// ----

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_grammar<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        constexpr auto               first_production     = STRUCTURALIZE(WrappedValue.unwrap()->template get<1>());
        static constexpr std::size_t num_more_productions = WrappedValue.unwrap()->template get<2>().size();
        return []<std::size_t... Is>(std::index_sequence<Is...>)
        {
            return parsely::detail::make_grammar(first_production,
                                                 STRUCTURALIZE(WrappedValue.unwrap()
                                                                   ->template get<2>()[Is]
                                                                   .template get<1>())...);
        }(std::make_index_sequence<num_more_productions>{});
    }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_production<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        static constexpr auto symbol     = WrappedValue.unwrap()->template get<0>().source_text;
        static constexpr auto expression = STRUCTURALIZE(WrappedValue.unwrap()->template get<4>());
        return parsely::detail::production{structural::inplace_string<symbol.size()>{symbol}, expression};
    }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_expression<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize() { return STRUCTURALIZE(*WrappedValue.unwrap()); }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_alt_expr<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        static constexpr auto first_seq_expr     = STRUCTURALIZE(WrappedValue.unwrap()->template get<0>());
        static constexpr auto more_seq_expr_size = WrappedValue.unwrap()->template get<1>().size();

        return []<std::size_t... Is>(std::index_sequence<Is...>)
        {
            if constexpr (more_seq_expr_size > 0)
                return parsely::detail::make_alt_expr(first_seq_expr,
                                                      STRUCTURALIZE(WrappedValue.unwrap()
                                                                        ->template get<1>()[Is]
                                                                        .template get<3>())...);
            else
                return first_seq_expr;
        }(std::make_index_sequence<more_seq_expr_size>{});
    }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_seq_expr<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        static constexpr auto first_prim_expr     = STRUCTURALIZE(WrappedValue.unwrap()->template get<0>());
        static constexpr auto more_prim_expr_size = WrappedValue.unwrap()->template get<1>().size();

        return []<std::size_t... Is>(std::index_sequence<Is...>)
        {
            if constexpr (more_prim_expr_size > 0)
                return parsely::detail::make_seq_expr(first_prim_expr,
                                                      STRUCTURALIZE(WrappedValue.unwrap()
                                                                        ->template get<1>()[Is]
                                                                        .template get<1>())...);
            else
                return first_prim_expr;
        }(std::make_index_sequence<more_prim_expr_size>{});
    }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_prim_expr<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        if constexpr (WrappedValue.unwrap()->index() == 0)
            return STRUCTURALIZE(WrappedValue.unwrap()->template get<0>());
        else if constexpr (WrappedValue.unwrap()->index() == 1)
            return STRUCTURALIZE(WrappedValue.unwrap()->template get<1>());
        else
            return STRUCTURALIZE(WrappedValue.unwrap()->template get<2>());
    }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_paren_expr<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize() { return STRUCTURALIZE(WrappedValue.unwrap()->template get<1>()); }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_terminal<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        static constexpr auto terminal = WrappedValue.unwrap()->template get<1>().source_text;
        return parsely::detail::terminal_expr{structural::inplace_string<terminal.size()>{terminal}};
    }
};

template<inplace_string GrammarDescription, wrapper WrappedValue>
struct structuralizer<grammar_parse_tree_node_nonterminal<GrammarDescription>, WrappedValue>
{
    static consteval auto do_structuralize()
    {
        static constexpr auto symbol = WrappedValue.unwrap().source_text;
        return parsely::detail::nonterminal_expr{structural::inplace_string<symbol.size()>{symbol}};
    }
};
} // namespace structural

#endif // INCLUDE_PARSELY_UTILITY_GRAMMAR_PARSER_HPP
