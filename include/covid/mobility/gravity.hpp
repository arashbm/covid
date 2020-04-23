#ifndef INCLUDE_COVID_MOBILITY_GRAVITY_HPP_
#define INCLUDE_COVID_MOBILITY_GRAVITY_HPP_

#include <optional>
#include <unordered_map>
#include <vector>
#include <string>

namespace covid {
  namespace mobility {
    class empty_class {};

    class gravity {
    public:
      // ignores hint
      using hint_type = empty_class;

      explicit gravity(const std::string& filename);

      std::pair<std::vector<std::pair<std::string, double>>, hint_type>
      trips_to(const std::string& destination,
          double t, double dt,
          std::optional<hint_type> hint = std::nullopt) const;

      std::pair<std::vector<std::pair<std::string, double>>, hint_type>
      trips_from(const std::string& origin,
          double t, double dt,
          std::optional<hint_type> hint = std::nullopt) const;

    private:
      std::unordered_map<std::string,
        std::vector<std::pair<std::string, double>>> in, out;

      double distance(
          std::tuple<double, double, double> a,
          std::tuple<double, double, double> b) const;
    };
  }  // namespace mobility
}  // namespace covid

#endif  // INCLUDE_COVID_MOBILITY_GRAVITY_HPP_
