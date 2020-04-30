#include <unordered_map>
#include <vector>
#include <algorithm>
#include <limits>
#include <iostream>
#include <string>
#include <numeric>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#pragma GCC diagnostic pop

#include "../include/covid/mobility/traffic.hpp"

namespace covid {
  namespace mobility {
    traffic::traffic(
        const std::string& mobility_filename,
        const std::string& population_filename) {
      std::unordered_map<std::string, double> population;
      for (auto&& row: csv::CSVReader(population_filename))
        population.try_emplace(
            row["Area"].get<>(),
            row["Total population"].get<double>());

      std::vector<std::string> col_names = {
        "origin", "destination", "time", "volume"};
      csv::CSVFormat format;
      format.column_names(col_names).delimiter(' ').quote('"');

      csv::CSVReader reader(mobility_filename, format);
      for (auto&& row: reader) {
        if (!row.empty()) {
          in[row["destination"].get<>()].emplace_back(row["time"].get<double>(),
              row["origin"].get<>(), row["volume"].get<double>());
          out[row["origin"].get<>()].emplace_back(row["time"].get<double>(),
              row["destination"].get<>(), row["volume"].get<double>());
        }
      }

      std::unordered_map<std::string, std::unordered_map<double, double>>
        trafic_population;
      for (auto& [k, v]: out) {
        double pop = 0.0;
        double current_time = 0.0;
        for (auto& [time, dest, volume]: v) {
          if (time > current_time) {
            trafic_population[k].emplace(current_time, pop);
            pop = 0.0;
          }
          pop += volume;
        }
      }

/*       for (auto& [k, v]: population) { */
/*         std::cerr << k << " telia: " << trafic_population[k][0.0] */
/*           << " pop: " << v << " relative pop: " << */
/*           trafic_population[k][0.0]/v << std::endl; */
/*       } */

      for (auto& [k, v]: in) {
        v.erase(std::remove_if(v.begin(), v.end(),
              [&k]
              (const std::tuple<double, std::string, double>& item) -> bool {
                return k == std::get<1>(item);
              }), v.end());
        std::transform(v.begin(), v.end(), v.begin(),
            [&trafic_population, &population]
            (const std::tuple<double, std::string, double>& item) {
              auto& [time, origin, volume] = item;
              return std::make_tuple(time, origin,
                        volume*population[origin]/
                        trafic_population[origin][time]);
            });
        v.erase(std::remove_if(v.begin(), v.end(),
              [](const std::tuple<double, std::string, double>& item) -> bool {
                return std::get<2>(item) < 0.5;
              }), v.end());
        std::sort(v.begin(), v.end());
      }

      for (auto& [k, v]: out) {
        v.erase(std::remove_if(v.begin(), v.end(),
              [&k]
              (const std::tuple<double, std::string, double>& item) -> bool {
                return k == std::get<1>(item);
              }), v.end());
        std::transform(v.begin(), v.end(), v.begin(),
            [&trafic_population, &population, &k]
            (const std::tuple<double, std::string, double>& item) {
              auto& [time, destination, volume] = item;
              return std::make_tuple(time, destination,
                      volume*population[k]/
                      trafic_population[k][time]);
            });
        v.erase(std::remove_if(v.begin(), v.end(),
              [](const std::tuple<double, std::string, double>& item) -> bool {
                return std::get<2>(item) < 0.5;
              }), v.end());
        std::sort(v.begin(), v.end());
      }
    }

    std::pair<std::vector<std::pair<std::string, double>>, traffic::hint_type>
    traffic::trips_to(
        const std::string& destination,
        double t, double dt,
        std::optional<hint_type> hint) const {
      if (in.find(destination) == in.end())
        return {{}, {}};
      else
        return trips(t, dt,
            in.at(destination).begin(), in.at(destination).end(), hint);
    }

    std::pair<std::vector<std::pair<std::string, double>>, traffic::hint_type>
    traffic::trips_from(
        const std::string& origin, double t, double dt,
        std::optional<hint_type> hint) const {
      if (out.find(origin) == out.end())
        return {{}, {}};
      else
        return trips(t, dt,
            out.at(origin).begin(), out.at(origin).end(), hint);
    }

    std::pair<std::vector<std::pair<std::string, double>>, traffic::hint_type>
    traffic::trips(
        double t, double dt,
        std::vector<
          std::tuple<double, std::string, double>>::const_iterator begin,
        std::vector<
          std::tuple<double, std::string, double>>::const_iterator end,
        std::optional<hint_type> hint) const {
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
      res.reserve(end - begin);
      std::transform(begin, end, std::back_inserter(res),
          [](std::tuple<double, std::string, double> item){
            auto &[time, id, trips] = item;
            return std::make_pair(id, trips);
      });

      return {res, end};
    }
  }  // namespace mobility
}  // namespace covid
