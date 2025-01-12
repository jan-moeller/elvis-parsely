//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#ifndef STRING_HPP
#define STRING_HPP

#include <algorithm>
#include <array>

namespace parsely
{
// Checks whether a character is one of the blank characters ' ', '\t'
constexpr auto is_blank(char const c) -> bool
{
    static constexpr std::array chars = {' ', '\t'};
    return std::ranges::contains(chars, c);
}

// Checks whether a character is one of the whitespace characters ' ', '\t', '\n', '\r', '\v', '\f'
constexpr auto is_space(char const c) -> bool
{
    static constexpr std::array chars = {'\n', '\r', '\v', '\f'};
    return is_blank(c) || std::ranges::contains(chars, c);
}

// Checks whether a character is one of the decimal digit characters '0' through '9'
constexpr auto is_digit(char const c) -> bool
{
    return c >= '0' && c <= '9';
}

// Checks whether a character is one of the latin alphabetic characters, either uppercase or lowercase
constexpr auto is_alpha(char const c) -> bool
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

// Equivalent to `is_alpha(c) || is_digit(c)`
constexpr auto is_alnum(char const c) -> bool
{
    return is_alpha(c) || is_digit(c);
}
} // namespace parsely

#endif // STRING_HPP
