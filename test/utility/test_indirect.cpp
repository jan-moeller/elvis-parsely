//
// Elvis Parsely
// Copyright (c) 2024 Jan Möller.
//

#include <parsely/utility/indirect.hpp>

#include <catch2/catch_all.hpp>

using namespace parsely;

TEST_CASE("indirect")
{
    SECTION("construction")
    {
        indirect<int> i0;
        indirect<int> i1 = indirect<int>(nullptr);
        indirect<int> i2 = 42;
        indirect<int> i3 = indirect(std::make_unique<int>(42));
        CHECK(!i0);
        CHECK(!i1);
        CHECK(i2 == 42);
        CHECK(i3 == 42);
    }
    SECTION("assignment")
    {
        indirect<int> i0;
        indirect<int> i1 = indirect<int>(nullptr);
        indirect<int> i2 = 42;
        indirect<int> i3 = indirect(std::make_unique<int>(42));

        i3 = nullptr;
        i2 = 0;
        i1 = i2;
        i0 = 42;

        CHECK(!i3);
        CHECK(i2 == 0);
        CHECK(i1 == 0);
        CHECK(i0 == 42);
    }
    SECTION("swap")
    {
        indirect<int> i0;
        indirect<int> i1 = 42;

        i0.swap(i1);

        CHECK(i0 == 42);
        CHECK(i1 == nullptr);
    }
}