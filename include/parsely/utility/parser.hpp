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

#include <structural/inplace_string.hpp>

namespace parsely
{
namespace detail
{
template<structural::inplace_string Grammar>
constexpr auto create_failure_string(auto /*parse_tree*/) -> std::string
{
    // TODO: Implement dynamic error string creation
    return "The grammar is invalid.";
}
template<structural::inplace_string Grammar>
consteval auto parse_grammar()
{
    static_assert(grammar_parser<Grammar>::parse().valid,
                  create_failure_string<Grammar>(grammar_parser<Grammar>::parse()));
    return STRUCTURALIZE(grammar_parser<Grammar>::parse());
}
} // namespace detail

// A parser for the given grammar
template<structural::inplace_string Grammar>
struct parser
{
  private:
    static constexpr auto        s_grammar         = detail::parse_grammar<Grammar>();
    static constexpr std::size_t s_num_productions = std::tuple_size_v<decltype(s_grammar.productions)>;

    template<typename, auto>
    friend struct parse_tree_node;

    template<typename Parser, detail::nonterminal_expr Expr>
    friend constexpr auto detail::parse_nonterminal(std::string_view input) -> parse_tree_node<Parser, Expr>;

  public:
    // Parses the given input string and returns a parse tree
    //
    // The Symbol NTTP indicates which production to use for parsing. By default, the production first mentioned in the
    // grammar is used.
    template<structural::inplace_string Symbol = get<0>(s_grammar.productions).symbol>
    static constexpr auto parse(std::string_view const input)
        -> parse_tree_node<parser, detail::nonterminal_expr{Symbol}>
    {
        return detail::parse_nonterminal<parser, detail::nonterminal_expr{Symbol}>(input);
    }

    // Parses the given input string
    static constexpr auto operator()(std::string_view const input) { return parse<>(input); }
};
} // namespace parsely

#endif // GRAMMAR_PARSER_HPP
