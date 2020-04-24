#include <cmath>

#include "../include/covid/geo.hpp"

namespace covid {
  namespace geo {
    lla::lla(long double lat, long double lon, long double alt)
        : latitude(lat), longitude(lon), altitude(alt) {}
    lla::lla()
        : latitude(0.0L), longitude(0.0L), altitude(0.0L) {}

    lla::lla(const ecef& ec) {
      // Ferrari's solution
      long double r = std::sqrt(ec.x*ec.x + ec.y*ec.y);
      long double f = 54.0L*datum::b*datum::b*ec.z*ec.z;
      long double g = r*r + (1.0L - datum::e_squared)*(ec.z*ec.z)
        - datum::e_squared*(datum::a*datum::a - datum::b*datum::b);
      long double c = (datum::e_squared*datum::e_squared)*f*r*r/(g*g*g);
      long double s = std::cbrt(1.0L + c + std::sqrt(c*c + 2.0L*c));
      long double p = f/(3.0L*std::pow(s + 1.0L/s + 1.0L, 2.0L)*g*g);
      long double q = std::sqrt(1.0L
          + 2.0L*datum::e_squared*datum::e_squared*p);
      long double r0 = -p*datum::e_squared*r/(1.0L + q)
        + std::sqrt(
            + 1.0L/2.0L*datum::a*datum::a*(1.0L + 1.0L/q)
            - p*(1.0L - datum::e_squared)*ec.z*ec.z/(q*(1.0L + q))
            - 1.0L/2.0L*p*r*r);
      long double u = std::sqrt(
          + std::pow(r - datum::e_squared*r0, 2.0L)
          + ec.z*ec.z);
      long double v = std::sqrt(
          + std::pow(r - datum::e_squared*r0, 2.0L)
          + (1.0L - datum::e_squared)*ec.z*ec.z);
      long double z0 = (datum::b*datum::b*ec.z)/(datum::a*v);

      altitude = u*(1.0L - (datum::b*datum::b)/(datum::a*v));
      latitude = std::atan((ec.z + datum::e_prime_squared*z0)/r)*
        180.0L*datum::rec_pi;
      longitude = std::atan2(ec.y, ec.x)*180.0L*datum::rec_pi;
    }

    long double lla::prime_vertical_radius() const {
      return datum::a/
        std::sqrt(1.0L
            - datum::e_squared*std::pow(std::sin(latitude*datum::pi/180.0L),
              2.0L));
    }

    ecef::ecef(long double ix, long double iy, long double iz)
      : x(ix), y(iy), z(iz) {}
    ecef::ecef() : x(0.0L), y(0.0L), z(0.0L) {}
    ecef::ecef(const lla& latlong) {
      long double center_alt = latlong.prime_vertical_radius()+latlong.altitude;
      x = center_alt*
        std::cos(latlong.latitude*datum::pi/180.0L)*
        std::cos(latlong.longitude*datum::pi/180.0L);
      y = center_alt*
        std::cos(latlong.latitude*datum::pi/180.0L)*
        std::sin(latlong.longitude*datum::pi/180.0L);
      z = (((datum::b*datum::b)/(datum::a*datum::a))*
          latlong.prime_vertical_radius()+latlong.altitude)*
        std::sin(latlong.latitude*datum::pi/180.0L);
    }
  }  // namespace geo
}  // namespace covid
