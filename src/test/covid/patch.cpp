#include "catch.hpp"

#include "../../../include/covid/config.hpp"
#include "../../../include/covid/patch.hpp"

using Catch::Matchers::WithinAbs;

TEST_CASE("patch", "[covid::patch]") {
  covid::config conf;
  conf.pi = 1.0/2.0;
  conf.eta = 1.0/2.34;
  conf.theta = 0.05;
  conf.nu = 1/2.86;
  conf.rho = 1.0/2.0;
  conf.chi = 1.0/10.0;
  conf.delta = 1.0/7.0;
  conf.delta = 1.0/5.0;
  conf.beta_infected = 0.06;
  conf.beta_asymptomatic = 0.06;
  conf.beta_presymptomatic = 0.06;
  conf.kappa = 0.2;
  conf.contact = covid::ContactMatrixType({{
    {0.5, 0.4, 0.1},
    {0.3, 0.6, 0.1},
    {0.2, 0.6, 0.2}}});

  SECTION("no infectious person in population") {
    covid::patch patch({
        // S,  E,  A,  P,  I,  H,    D,    R
        {100., 0., 0., 0., 0., 100., 100., 100.},  // Young
        {100., 0., 0., 0., 0., 100., 100., 100.},  // Adults
        {100., 0., 0., 0., 0., 100., 100., 100.},  // Elderly
      });

    auto delta = patch.delta_population(conf);

    for (auto&& g: covid::all_age_groups) {
        REQUIRE_THAT(
            delta[g][covid::compartments::exposed],
            WithinAbs(0.0, 1e-5));

        REQUIRE_THAT(
            delta[g][covid::compartments::asymptomatic],
            WithinAbs(0.0, 1e-5));

        REQUIRE_THAT(
            delta[g][covid::compartments::infected],
            WithinAbs(0.0, 1e-5));
    }
  }

  SECTION("R0 is zero") {
    conf.beta_infected = 0.0;
    conf.beta_asymptomatic = 0.0;
    conf.beta_presymptomatic = 0.0;

    covid::patch patch({
        // S,  E,  A,   P,   I,   H,  D,  R
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Young
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Adults
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Elderly
      });

    auto delta = patch.delta_population(conf);

    for (auto&& g: covid::all_age_groups) {
        REQUIRE_THAT(
            delta[g][covid::compartments::exposed],
            WithinAbs(0.0, 1e-5));
    }
  }

  SECTION("full distancing") {
    conf.contact = covid::ContactMatrixType({{
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0},
      {0.0, 0.0, 0.0}}});

    covid::patch patch({
        // S,  E,  A,   P,   I,   H,  D,  R
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Young
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Adults
        {100., 0., 10., 10., 10., 0., 0., 0.},  // Elderly
      });

    auto delta = patch.delta_population(conf);

    for (auto&& g: covid::all_age_groups) {
        REQUIRE_THAT(
            delta[g][covid::compartments::exposed],
            WithinAbs(0.0, 1e-5));
    }
  }
}
