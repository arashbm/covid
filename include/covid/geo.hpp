#ifndef INCLUDE_COVID_GEO_HPP_
#define INCLUDE_COVID_GEO_HPP_

namespace covid {
  namespace geo {
    namespace datum {
      constexpr long double a = 6378137.0L;
      constexpr long double rec_f = 298.257223563L;
      constexpr long double b = 6356752.314245L;
      constexpr long double e_squared = 6.69437999014e-3L;
      constexpr long double e_prime_squared = 6.73949674228e-3L;
      constexpr long double pi = 3.141592653589793238L;
      constexpr long double rec_pi = 0.31830988618379067153L;
    }  // namespace datum

    struct ecef;

    struct lla {
      long double latitude, longitude, altitude;

      lla(long double lat, long double lon, long double alt=0.0L);
      lla();
      explicit lla(const ecef& ec);

      // prime vertical radius of curvature N(phi):
      // distance from the surface to the Z-axis along the ellipsoid normal at
      // current latitude
      long double prime_vertical_radius() const;
    };

    struct ecef {
      long double x, y, z;

      ecef(long double ix, long double iy, long double iz);
      ecef();
      explicit ecef(const lla& latlong);
    };
  }  // namespace geo
}  // namespace covid

#endif  // INCLUDE_COVID_GEO_HPP_
