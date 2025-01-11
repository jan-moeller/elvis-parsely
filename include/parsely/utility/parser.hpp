//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef GRAMMAR_PARSER_HPP
#define GRAMMAR_PARSER_HPP

#include <parsely/utility/grammar_ast.hpp>
#include <parsely/utility/grammar_parser.hpp>
#include <parsely/utility/indirect.hpp>
#include <parsely/utility/parse_tree_node.hpp>
#include <parsely/utility/parser_creator.hpp>
#include <parsely/utility/string.hpp>

#include <structural/inplace_string.hpp>

namespace parsely
{
namespace detail
{
template<structural::inplace_string Grammar>
constexpr auto is_parse_result_valid(auto parse_tree) -> bool
{
    return parse_tree.valid && parse_tree.source_text == Grammar;
}
template<structural::inplace_string Grammar>
constexpr auto create_failure_string(auto /*parse_tree*/) -> std::string
{
    // TODO: Implement dynamic error string creation
    return "The grammar is invalid.";
}
} // namespace detail

// A parser for the given grammar
template<structural::inplace_string Grammar>
struct parser
{
  private:
    static constexpr auto s_grammar = []
    {
        static_assert(detail::is_parse_result_valid<Grammar>(detail::grammar_parser<Grammar>::parse()),
                      detail::create_failure_string<Grammar>(detail::grammar_parser<Grammar>::parse()));
        return STRUCTURALIZE(detail::grammar_parser<Grammar>::parse());
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
    static constexpr auto parse(std::string_view const input)
        -> parse_tree_node<parser, detail::nonterminal_expr{Symbol}>
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
        return parse_tree_node<parser, detail::nonterminal_expr{Symbol}>{
            .valid       = result.valid,
            .source_text = result.source_text,
            .nested      = indirect(std::move(result)),
        };
    }

    // Parses the given input string
    static constexpr auto operator()(std::string_view const input) { return parse<>(input); }
};
} // namespace parsely

#endif // GRAMMAR_PARSER_HPP
