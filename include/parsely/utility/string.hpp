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
} // namespace parsely

#endif // STRING_HPP
