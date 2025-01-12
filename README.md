# Elvis Parsely

An experimental C++26 constexpr parser generator.

[![gcc](https://github.com/jan-moeller/elvis-parsely/actions/workflows/gcc.yml/badge.svg)](https://github.com/jan-moeller/elvis-parsely/actions/workflows/gcc.yml)

## Description

Elvis Parsely generates a parser from a compile-time string in BNF-like syntax. That parser is a simple recursive decent
parser, and therefore doesn't support left recursion. The parsing result is a strongly typed parse tree.

## General Usage

```c++
constexpr structural::inplace_string grammar = R"raw(
    expr: binary_expr | unary_expr;
    binary_expr: unary_expr binop expr;
    unary_expr: unop prim_expr | prim_expr;
    prim_expr: "(" expr ")" | number;
    number: digit number | digit;

    digit: "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9";
    unop: "+" | "-";
    binop: "+" | "-" | "*" | "/";
)raw";

constexpr parser<grammar> parse;

auto const parse_tree = parse("-(1+2)*3");
```

## Grammar

The language to parse is described as a list of productions of the form

```
<symbol> : <expression> ;
```

Here, `<symbol>` is any text containing only alphanumeric characters and `"_"`. `<expression>` can contain any of the
following constructs:

* `"<terminal>"`: matches `<terminal>` literally.
* `<symbol>`: matches the production named `<symbol>`.
* `<expression_1> <expression_2> ...`: Matches `<expression_1>` followed by `<expression_2>` etc.
* `<expression_1> | <expression_2> ...`: Matches `<expression_1>`. If it fails to parse, matches `<expression_2>` etc.

## Parse Tree Types

The parse tree consists of types following these patterns:

### Terminal nodes (generated from `"<terminal>"` expressions)

```c++
template</* implementation detail */>
struct parse_tree_node</* ... */>
{
    static constexpr std::string_view terminal = /* depends on grammar */;

    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };
};
```

### Symbol nodes (generated from `<symbol>`) expressions

```c++
template</* implementation detail */>
struct parse_tree_node</* ... */>
{
    using nested_type = indirect<parse_tree_node</* depends on grammar */>>;

    static constexpr std::string_view symbol = /* depends on grammar */;

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
```

### Sequence nodes (generated from `<expression_1> <expression_2> ...` expressions)

```c++
template</* implementation detail */>
struct parse_tree_node</* ... */>
{
    using nested_type = std::tuple<parse_tree_node</* depends on grammar */>, /* ... */>;

    bool             valid = false; // True if parsing successful
    std::string_view source_text;   // Consumed source text
    nested_type      node_sequence; // Tuple of more parse_tree_nodes

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };

    template<std::size_t I>
    constexpr auto get() -> decltype(auto);
    template<std::size_t I>
    constexpr auto get() const -> decltype(auto);
};
```

### Alternative nodes (generated from `<expression_1> | <expression_2> ...` expressions)

```c++
template</* implementation detail */>
struct parse_tree_node</* ... */>
{
    using nested_type = std::variant<parse_tree_node</* depends on grammar */>, /* ... */>;

    bool             valid = false;     // True if parsing successful
    std::string_view source_text;       // Consumed source text
    nested_type      node_alternatives; // Variant containing the matched parse_tree_node

    constexpr auto operator==(parse_tree_node const&) const -> bool = default;

    constexpr explicit operator bool() const { return valid; };

    template<class Self, class Visitor>
    constexpr auto visit(this Self&& self, Visitor&& vis) -> decltype(auto);
    template<class R, class Self, class Visitor>
    constexpr auto visit(this Self&& self, Visitor&& vis) -> R;

    template<std::size_t I>
    constexpr auto get() -> decltype(auto);
    template<std::size_t I>
    constexpr auto get() const -> decltype(auto);
};
```

## To Do

- Error out on left recursive grammars at compile time.
- Better error reporting.
- Support extended BNF constructs. 
- Support non-char strings (unicode?)
- Provide some common terminals as build-ins
