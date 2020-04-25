#include "catch.hpp"

#include "../../../include/covid/models/arenas/metapop.hpp"
#include "../../../include/covid/models/arenas/patch.hpp"

using Catch::Matchers::WithinAbs;

namespace arenas = covid::models::arenas;

TEST_CASE("arenas::patch", "[covid::models::arenas::patch]") {
  arenas::patch::config_type conf;
  conf.eta   = arenas::age_array_type<double>{1.0/2.34, 1.0/2.34, 1.0/2.34};
  conf.alpha = arenas::age_array_type<double>{1.0/5.06, 1.0/2.86, 1.0/2.86};
  conf.mu    = arenas::age_array_type<double>{1.0/1.0,  1.0/3.2,  1.0/3.2};
  conf.psi   = arenas::age_array_type<double>{1.0/7.0,  1.0/7.0,  1.0/7.0};
  conf.chi   = arenas::age_array_type<double>{1.0/10.0, 1.0/10.0, 1.0/10.0};

  conf.omega = arenas::age_array_type<double>{0.42, 0.42, 0.42};
  conf.gamma = arenas::age_array_type<double>{2e-3, 0.05, 0.36};
  conf.p     = arenas::age_array_type<double>{0.00, 1.00, 0.00};
  conf.k     = arenas::age_array_type<double>{11.0, 13.0, 6.60};
  conf.beta_infected = 0.06;
  conf.beta_asymptomatic = 0.06;
  conf.kappa = 0.22;
  conf.xi = 0.01;
  conf.contact = arenas::contact_matrix_type({{
    {0.5, 0.4, 0.1},
    {0.3, 0.6, 0.1},
    {0.2, 0.6, 0.2}}});

  arenas::patch::population_type in_population, out_population;
  arenas::age_array_type<double> normalization_factor = {1.0, 1.0, 1.0};
  arenas::age_array_type<double> external_pi = {0.0, 0.0, 0.0};

  SECTION("initialisation") {
    arenas::patch patch({
        {1., 2., 3., 4., 5., 6., 7.},
        {1., 2., 3., 4., 5., 6., 7.},
        {1., 2., 3., 4., 5., 6., 7.},
      }, 1.0);

    REQUIRE(patch.area() == 1.0);

    auto p = patch.population();
    for (auto&& g: magic_enum::enum_values<arenas::age_groups>()) {
      REQUIRE(p[g][arenas::compartments::susceptible]    == 1.0);
      REQUIRE(p[g][arenas::compartments::exposed]        == 2.0);
      REQUIRE(p[g][arenas::compartments::asymptomatic]   == 3.0);
      REQUIRE(p[g][arenas::compartments::infected]       == 4.0);
      REQUIRE(p[g][arenas::compartments::hospitalized]   == 5.0);
      REQUIRE(p[g][arenas::compartments::dead]           == 6.0);
      REQUIRE(p[g][arenas::compartments::recovered]      == 7.0);
    }
  }

  SECTION("delta == 0") {
    arenas::patch patch({
        {1., 2., 3., 4., 5., 6., 7.},
        {1., 2., 3., 4., 5., 6., 7.},
        {1., 2., 3., 4., 5., 6., 7.},
      }, 1.0);

    auto delta = patch.delta(conf, 1.0,
        in_population, out_population, normalization_factor, external_pi);

    for (auto&& g: magic_enum::enum_values<arenas::age_groups>())
      REQUIRE_THAT(delta[g].sum(), WithinAbs(0.0, 1e-10));
  }

  SECTION("no infectious person in population") {
    arenas::patch patch({
        // S,  E,  A,  I,  H,    D,    R
        {100., 0., 0., 0., 100., 100., 100.},  // Young
        {100., 0., 0., 0., 100., 100., 100.},  // Adults
        {100., 0., 0., 0., 100., 100., 100.},  // Elderly
      }, 1.0);

    auto delta = patch.delta(conf, 1.0,
        in_population, out_population, normalization_factor, external_pi);


    for (auto&& g: magic_enum::enum_values<arenas::age_groups>()) {
        REQUIRE_THAT(
            delta[g][arenas::compartments::exposed],
            WithinAbs(0.0, 1e-5));

        REQUIRE_THAT(
            delta[g][arenas::compartments::asymptomatic],
            WithinAbs(0.0, 1e-5));

        REQUIRE_THAT(
            delta[g][arenas::compartments::infected],
            WithinAbs(0.0, 1e-5));
    }
  }

  SECTION("R0 is zero") {
    conf.beta_infected = 0.0;
    conf.beta_asymptomatic = 0.0;

    arenas::patch patch({
        // S,  E,  A,   I,   H,  D,  R
        {100., 0., 10., 10., 0., 0., 0.},  // Young
        {100., 0., 10., 10., 0., 0., 0.},  // Adults
        {100., 0., 10., 10., 0., 0., 0.},  // Elderly
      }, 1.0);

    auto delta = patch.delta(conf, 1.0,
        in_population, out_population, normalization_factor, external_pi);

    for (auto&& g: magic_enum::enum_values<arenas::age_groups>()) {
        REQUIRE_THAT(
            delta[g][arenas::compartments::exposed],
            WithinAbs(0.0, 1e-5));
    }
  }

  SECTION("full distancing") {
    conf.contact = arenas::contact_matrix_type({{
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0}}});

    arenas::patch patch({
        // S,  E,  A,   I,   H,  D,  R
        {100., 0., 10., 10., 0., 0., 0.},  // Young
        {100., 0., 10., 10., 0., 0., 0.},  // Adults
        {100., 0., 10., 10., 0., 0., 0.},  // Elderly
      }, 1.0);

    auto delta = patch.delta(conf, 1.0,
        in_population, out_population, normalization_factor, external_pi);


    for (auto&& g: magic_enum::enum_values<arenas::age_groups>()) {
        REQUIRE_THAT(
            delta[g][arenas::compartments::exposed],
            WithinAbs(0.0, 1e-5));
    }
  }
}
