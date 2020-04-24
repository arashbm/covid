#include "catch.hpp"

#include "../../../include/covid/geo.hpp"

using Catch::Matchers::WithinAbs;

namespace geo = covid::geo;

TEST_CASE("lla", "[covid::geo::lla]") {
  SECTION("initialisation") {
    geo::ecef helsinki_ecef(2884141.58, 1341124.14, 5509931.29);
    geo::lla helsinki_lla(helsinki_ecef);

    REQUIRE_THAT(helsinki_lla.latitude,  WithinAbs(60.1699, 1e-7));
    REQUIRE_THAT(helsinki_lla.longitude, WithinAbs(24.9384, 1e-7));
    REQUIRE_THAT(helsinki_lla.altitude,  WithinAbs(16.0,    1e-2));
  }
}

TEST_CASE("ecef", "[covid::geo::ecef]") {
  SECTION("initialisation") {
    geo::lla helsinki_lla(60.1699, 24.9384, 16.0);
    geo::ecef helsinki_ecef(helsinki_lla);

    REQUIRE_THAT(helsinki_ecef.x, WithinAbs(2884141.58, 1e-1));
    REQUIRE_THAT(helsinki_ecef.y, WithinAbs(1341124.14, 1e-1));
    REQUIRE_THAT(helsinki_ecef.z, WithinAbs(5509931.29, 1e-1));
  }
}
