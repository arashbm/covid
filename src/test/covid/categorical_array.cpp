#include "catch.hpp"

#include "../../../include/covid/categorical_array.hpp"

enum class test_enums {
  first,
  second,
  third,
};

constexpr std::array<test_enums, 3> all_test_enums = {
  test_enums::first, test_enums::second, test_enums::third};


TEST_CASE("categorical_array", "[covid::categorical_array]") {
  covid::categorical_array<int, test_enums, all_test_enums.size()>
    t = {1, 2, 3};
  SECTION("initialisation") {
    REQUIRE(t[test_enums::first]  == 1);
    REQUIRE(t[test_enums::second] == 2);
    REQUIRE(t[test_enums::third]  == 3);
  }

  SECTION("sum") {
    REQUIRE(t.sum()  == 6);
  }

  SECTION("copy semantics") {
    auto t2 = t;

    t2[test_enums::second] = 62;
    t[test_enums::second]  = 42;

    REQUIRE(t2[test_enums::first]  == 1);
    REQUIRE(t2[test_enums::second] == 62);
    REQUIRE(t2[test_enums::third]  == 3);
  }

  SECTION("assignment") {
    t[test_enums::second] = 42;

    REQUIRE(t[test_enums::first]  == 1);
    REQUIRE(t[test_enums::second] == 42);
    REQUIRE(t[test_enums::third]  == 3);
  }

  SECTION("nested array") {
    covid::categorical_array<
      covid::categorical_array<int, test_enums, all_test_enums.size()>,
      test_enums, all_test_enums.size()>
        deep_t = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};

    SECTION("initialisation") {
      REQUIRE(deep_t[test_enums::second][test_enums::first]  == 4);
      REQUIRE(deep_t[test_enums::second][test_enums::second] == 5);
      REQUIRE(deep_t[test_enums::second][test_enums::third]  == 6);
    }

    SECTION("sum") {
      auto s = deep_t.sum();

      REQUIRE(s[test_enums::first]  == 12);
      REQUIRE(s[test_enums::second] == 15);
      REQUIRE(s[test_enums::third]  == 18);

      REQUIRE(s.sum()  == 45);
    }


    SECTION("assignment") {
      deep_t[test_enums::second][test_enums::third] = 42;

      REQUIRE(deep_t[test_enums::second][test_enums::first]  == 4);
      REQUIRE(deep_t[test_enums::second][test_enums::second] == 5);
      REQUIRE(deep_t[test_enums::second][test_enums::third]  == 42);
    }
  }

  SECTION("arithmetics") {
    SECTION("multiplication by number") {
      auto r1 = t * 2;
      REQUIRE(r1[test_enums::first]  == 2);
      REQUIRE(r1[test_enums::second] == 4);
      REQUIRE(r1[test_enums::third]  == 6);

      auto r2 = 2 * t;
      REQUIRE(r2[test_enums::first]  == 2);
      REQUIRE(r2[test_enums::second] == 4);
      REQUIRE(r2[test_enums::third]  == 6);
    }

    SECTION("division by number") {
      auto r1 = t / 2;

      REQUIRE(r1[test_enums::first]  == 0);
      REQUIRE(r1[test_enums::second] == 1);
      REQUIRE(r1[test_enums::third]  == 1);
    }

    SECTION("addition to array") {
      covid::categorical_array<int, test_enums, all_test_enums.size()>
        t2 = {3, 4, 5};
      auto r = t + t2;

      REQUIRE(r[test_enums::first]  == 4);
      REQUIRE(r[test_enums::second] == 6);
      REQUIRE(r[test_enums::third]  == 8);

      t2 += t;
      REQUIRE(t2[test_enums::first]  == 4);
      REQUIRE(t2[test_enums::second] == 6);
      REQUIRE(t2[test_enums::third]  == 8);
    }

    SECTION("deduction from array") {
      covid::categorical_array<int, test_enums, all_test_enums.size()>
        t2 = {5, 4, 3};
      auto r = t - t2;

      REQUIRE(r[test_enums::first]  == -4);
      REQUIRE(r[test_enums::second] == -2);
      REQUIRE(r[test_enums::third]  == 0);

      t2 -= t;
      REQUIRE(t2[test_enums::first]  == 4);
      REQUIRE(t2[test_enums::second] == 2);
      REQUIRE(t2[test_enums::third]  == 0);
    }
  }
}
