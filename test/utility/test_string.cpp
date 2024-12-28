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
    STATIC_CHECK(trim<"0123456", is_digit>() == "");
}
