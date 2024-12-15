//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#include <parsely/utility/string.hpp>

#include <catch2/catch_all.hpp>

using namespace parsely;

TEST_CASE("trim")
{
    STATIC_CHECK(trim<"">().empty());
    STATIC_CHECK(trim<"asd">() == "asd");
    STATIC_CHECK(trim<"  asd   ">() == "asd");
    STATIC_CHECK(trim<"0123asd456", is_digit>() == "asd");
}

TEST_CASE("split")
{
    STATIC_CHECK(split<"", ':'>() == std::tuple(""));
    STATIC_CHECK(split<"foo", ':'>() == std::tuple("foo"));
    STATIC_CHECK(split<":", ':'>() == std::tuple("", ""));
    STATIC_CHECK(split<"::", ':'>() == std::tuple("", "", ""));
    STATIC_CHECK(split<"foo:bar:baz", ':'>() == std::tuple("foo", "bar", "baz"));
}

TEST_CASE("split_once")
{
    STATIC_CHECK(split_once<"", ':'>() == std::tuple(""));
    STATIC_CHECK(split_once<"foo", ':'>() == std::tuple("foo"));
    STATIC_CHECK(split_once<":", ':'>() == std::tuple("", ""));
    STATIC_CHECK(split_once<"::", ':'>() == std::tuple("", ":"));
    STATIC_CHECK(split_once<"foo:bar:baz", ':'>() == std::tuple("foo", "bar:baz"));
}

TEST_CASE("split_production")
{
    STATIC_CHECK(split_production<"a:b">() == std::pair("a", "b"));
    STATIC_CHECK(split_production<" a : b ">() == std::pair("a", "b"));
    STATIC_CHECK(split_production<R"( a : b | ":")">() == std::pair("a", R"(b | ":")"));
}
