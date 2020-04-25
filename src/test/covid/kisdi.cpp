#include "catch.hpp"

#include "../../../include/covid/models/kisdi/metapop.hpp"
#include "../../../include/covid/models/kisdi/patch.hpp"

using Catch::Matchers::WithinAbs;

namespace kisdi = covid::models::kisdi;

TEST_CASE("kisdi::patch", "[covid::models::kisdi::patch]") {
  kisdi::patch::config_type conf;
  conf.pi = 1.0/2.0;
  conf.eta = 1.0/2.34;
  conf.theta = 0.05;
  conf.alpha = 1/3.0;
  conf.nu = 1/2.86;
  conf.rho = 1.0/2.0;
  conf.chi = 1.0/10.0;
  conf.delta = 1.0/5.0;
  conf.beta_infected = 0.06;
  conf.beta_asymptomatic = 0.06;
  conf.beta_presymptomatic = 0.06;
  conf.kappa = 0.2;
  conf.contact = kisdi::contact_matrix_type({{
    {0.5, 0.4, 0.1},
    {0.3, 0.6, 0.1},
    {0.2, 0.6, 0.2}}});

  SECTION("initialisation") {
    kisdi::patch patch({
        {1., 2., 3., 4., 5., 6., 7., 8.},
        {1., 2., 3., 4., 5., 6., 7., 8.},
        {1., 2., 3., 4., 5., 6., 7., 8.},
      });

    auto p = patch.population();

    for (auto&& g: magic_enum::enum_values<kisdi::age_groups>()) {
      REQUIRE(p[g][kisdi::compartments::susceptible]    == 1.0);
      REQUIRE(p[g][kisdi::compartments::exposed]        == 2.0);
      REQUIRE(p[g][kisdi::compartments::asymptomatic]   == 3.0);
      REQUIRE(p[g][kisdi::compartments::presymptomatic] == 4.0);
      REQUIRE(p[g][kisdi::compartments::infected]       == 5.0);
      REQUIRE(p[g][kisdi::compartments::hospitalized]   == 6.0);
      REQUIRE(p[g][kisdi::compartments::dead]           == 7.0);
      REQUIRE(p[g][kisdi::compartments::recovered]      == 8.0);
    }
  }

  SECTION("delta == 0") {
    kisdi::patch patch({
        {1., 2., 3., 4., 5., 6., 7., 8.},
        {1., 2., 3., 4., 5., 6., 7., 8.},
        {1., 2., 3., 4., 5., 6., 7., 8.},
      });

    auto delta = patch.delta(conf, 1.0);

    for (auto&& g: magic_enum::enum_values<kisdi::age_groups>())
      REQUIRE_THAT(delta[g].sum(), WithinAbs(0.0, 1e-10));
  }

  SECTION("no infectious person in population") {
    kisdi::patch patch({
        // S,  E,  A,  P,  I,  H,    D,    R
        {100., 0., 0., 0., 0., 100., 100., 100.},  // Young
        {100., 0., 0., 0., 0., 100., 100., 100.},  // Adults
        {100., 0., 0., 0., 0., 100., 100., 100.},  // Elderly
      });

    auto delta = patch.delta(conf, 1.0);

    for (auto&& g: magic_enum::enum_values<kisdi::age_groups>()) {
        REQUIRE_THAT(
            delta[g][kisdi::compartments::exposed],
            WithinAbs(0.0, 1e-5));

        REQUIRE_THAT(
            delta[g][kisdi::compartments::asymptomatic],
            WithinAbs(0.0, 1e-5));

        REQUIRE_THAT(
            delta[g][kisdi::compartments::infected],
            WithinAbs(0.0, 1e-5));
    }
  }

  SECTION("R0 is zero") {
    conf.beta_infected = 0.0;
    conf.beta_asymptomatic = 0.0;
    conf.beta_presymptomatic = 0.0;

    kisdi::patch patch({
        // S,  E,  A,   P,   I,   H,  D,  R
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Young
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Adults
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Elderly
      });

    auto delta = patch.delta(conf, 1.0);

    for (auto&& g: magic_enum::enum_values<kisdi::age_groups>()) {
        REQUIRE_THAT(
            delta[g][kisdi::compartments::exposed],
            WithinAbs(0.0, 1e-5));
    }
  }

  SECTION("full distancing") {
    conf.contact = kisdi::contact_matrix_type({{
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0}}});

    kisdi::patch patch({
        // S,  E,  A,   P,   I,   H,  D,  R
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Young
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Adults
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Elderly
      });

    auto delta = patch.delta(conf, 1.0);

    for (auto&& g: magic_enum::enum_values<kisdi::age_groups>()) {
        REQUIRE_THAT(
            delta[g][kisdi::compartments::exposed],
            WithinAbs(0.0, 1e-5));
    }
  }
}
