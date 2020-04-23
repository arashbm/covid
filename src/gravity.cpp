#include <unordered_map>
#include <vector>
#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#pragma GCC diagnostic pop

#include "../include/covid/mobility/gravity.hpp"

namespace covid {
  namespace mobility {
    gravity::gravity(const std::string& filename) {
      // TODO read these from some config file
      constexpr double total_trips_per_day = 5.5e6;
      constexpr double cutoff = 10;
      constexpr double exponent = 2;

      csv::CSVReader reader(filename);
      std::unordered_map<std::string, double> pop;
      std::unordered_map<std::string, std::tuple<double, double, double>> loc;

      for (auto&& row: reader) {
        std::string area(row["Area"].get<>());

        pop.try_emplace(
            area,
            row["Total population"].get<double>());

        loc.try_emplace(
            area,
            row["Mean X"].get<double>(),
            row["Mean Y"].get<double>(),
            row["Mean Z"].get<double>());

        in[area] = {};
        out[area] = {};
      }


      double normalization_factor = 0.0;
      for (auto &[i, pop_i]: pop)
        for (auto &[j, pop_j]: pop)
          if (i != j)
            normalization_factor +=
              pop_i*pop_j*std::pow(distance(loc.at(i), loc.at(j)), exponent);

      for (auto &[i, pop_i]: pop) {
        for (auto &[j, pop_j]: pop) {
          if (i != j) {
            double trips = total_trips_per_day*
              pop_i*pop_j*std::pow(distance(loc.at(i), loc.at(j)), exponent)/
              normalization_factor;
            if (trips > cutoff) {
              in[i].emplace_back(j, trips);
              out[j].emplace_back(i, trips);
            }
          }
        }
      }
    }

    std::pair<std::vector<std::pair<std::string, double>>, gravity::hint_type>
    gravity::trips_to(
        const std::string& destination,
        double /* t */, double dt,
        std::optional<hint_type> /* hint */) const {
      auto begin = in.at(destination).begin(), end = in.at(destination).end();
      std::vector<std::pair<std::string, double>> res;
      res.reserve(end - begin);
      std::transform(begin, end, std::back_inserter(res),
          [dt](std::tuple<std::string, double> item){
            auto &[id, trips] = item;
            return std::make_pair(id, trips*dt);
          });
      return {res, empty_class{}};
    }

    std::pair<std::vector<std::pair<std::string, double>>, gravity::hint_type>
    gravity::trips_from(
        const std::string& origin,
        double /* t */, double dt,
        std::optional<hint_type> /* hint */) const {
      auto begin = out.at(origin).begin(), end = out.at(origin).end();
      std::vector<std::pair<std::string, double>> res;
      res.reserve(end - begin);
      std::transform(begin, end, std::back_inserter(res),
          [dt](std::pair<std::string, double> item){
            auto &[id, trips] = item;
            return std::make_pair(id, trips*dt);
          });
      return {res, empty_class{}};
    }

    double
    gravity::distance(std::tuple<double, double, double> a,
      std::tuple<double, double, double> b) const {
      return std::sqrt(
          std::pow(std::get<0>(b) - std::get<0>(a), 2.0) +
          std::pow(std::get<1>(b) - std::get<1>(a), 2.0) +
          std::pow(std::get<2>(b) - std::get<2>(a), 2.0));
    }
  }  // namespace mobility
}  // namespace covid
