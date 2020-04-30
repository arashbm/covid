#ifndef INCLUDE_COVID_MOBILITY_TRAFFIC_HPP_
#define INCLUDE_COVID_MOBILITY_TRAFFIC_HPP_

#include <optional>
#include <unordered_map>

namespace covid {
  namespace mobility {
    class traffic {
    public:
      using hint_type =
        std::vector<std::tuple<double, std::string, double>>::const_iterator;

      explicit traffic(
          const std::string& mobility_filename,
          const std::string& population_filename);

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
        std::vector<std::tuple<double, std::string, double>>> in, out;

      std::pair<std::vector<std::pair<std::string, double>>, hint_type>
      trips(double t, double dt,
          std::vector<
            std::tuple<double, std::string, double>>::const_iterator begin,
          std::vector<
            std::tuple<double, std::string, double>>::const_iterator end,
          std::optional<hint_type> hint = std::nullopt) const;
    };
  }  // namespace mobility
}  // namespace covid

#endif  // INCLUDE_COVID_MOBILITY_TRAFFIC_HPP_
