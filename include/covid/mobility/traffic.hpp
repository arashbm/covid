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

      explicit traffic(std::string filename);

      std::pair<std::vector<std::pair<std::string, double>>, hint_type>
      trips_to(const std::string& destination,
          double t, double dt,
          std::optional<hint_type> hint = std::nullopt) const;

      std::pair<std::vector<std::pair<std::string, double>>, hint_type>
      trips_from(const std::string& origin,
          double t, double dt,
          std::optional<hint_type> hint = std::nullopt) const;

    private:
      unordered_map<std::string,
        std::vector<std::tuple<double, std::string, double>>> in, out;

      std::pair<std::vector<std::pair<std::string, double>>, hint_type>
      trips(const std::string& id,
          double t, double dt,
          in::iterator begin, in::iterator out,
          std::optional<hint_type> hint = std::nullopt) const;
    };
  }  // namespace mobility
}  // namespace covid

#endif  // INCLUDE_COVID_MOBILITY_TRAFFIC_HPP_
