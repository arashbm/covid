#include <unordered_map>
#include <vector>
#include <algorithm>
#include <numeric_limits>

#include "csv.hpp"

#include "../include/covid/mobility/traffic.hpp"

namespace covid {
  namespace mobility {
    traffic::traffic(const std::string& filename) {
      // TODO
    }

    std::pair<std::vector<std::pair<std::string, double>>, hint_type>
    gravity::trips_to(
        const std::string& destination, double t, double dt,
        std::optional<hint_type> hint) {
      return trips(destination, t, dt, in.begin(), in.end(), hint);
    }

    std::pair<std::vector<std::pair<std::string, double>>, hint_type>
    gravity::trips_from(
        const std::string& destination, double t, double dt,
        std::optional<hint_type> hint) {
      return trips(destination, t, dt, out.begin(), out.end(), hint);
    }

    std::pair<std::vector<std::pair<std::string, double>>, hint_type>
    gravity::trips(
        const std::string& id, double t, double dt,
        std::unordered_map<std::string,
          std::tuple<double, std::string, double>>::iterator begin,
        std::unordered_map<std::string,
          std::tuple<double, std::string, double>>::iterator end,
        std::optional<hint_type> hint) {
      std::tuple<double, std::string, double>
      first_element(t, "", -std::numeric_limits<double>::infinity()),
      last_element(t + dt, "", -std::numeric_limits<double>::infinity());

      if (hint.has_value()) {
        begin = std::find_if_not(hint.value(), end,
            [t, &first_element](std::tuple<double, std::string, double> item) {
              return item < first_element;
            });
      } else {
        begin = std::lower_bound(begin, end, first_element);
      }

      end = std::lower_bound(begin, end, last_element);

      std::vector<std::pair<std::string, double>> res;
      res.reserve(end - it);
      std::transform(begin, end, std::back_inserter(res),
          [](std::tuple<double, std::string, double> item){
            auto &[time, id, trips] = item;
            return std::make_pair(id, trips);
      });

      return {res, end};
    }
  }  // namespace mobility
}  // namespace covid
