#include <random>
#include <algorithm>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#include "magic_enum.hpp"
#pragma GCC diagnostic pop

#include "../include/covid/models/fhi/patch.hpp"
#include "../include/covid/models/fhi/config.hpp"

namespace covid {
  namespace models {
    namespace fhi {
      config::config(const std::string& model_config_filename) {
        csv::CSVReader reader(model_config_filename);
        for (auto&& row: reader) {
          std::string n = row[0].get<>();
          double v = row[1].get<double>();
          if (n == "beta") beta = v;
          else if (n == "p_a") p_a = v;
          else if (n == "r_a") r_a = v;
          else if (n == "r_p") r_p = v;
          else if (n == "reciprocal_lambda_1") lambda_1 = 1.0/v;
          else if (n == "reciprocal_lambda_2") lambda_2 = 1.0/v;
          else if (n == "reciprocal_gamma") gamma = 1.0/v;
        }
      }

      patch::patch(const population_type& population)
        : _population(population) {}

      const patch::population_type& patch::population() const {
        return _population;
      }

      double patch::force_of_infection(
          const config_type& c,
          const population_type& commuting_in_population) const {
        population_type p = commuting_in_population + _population;
        double n_i = static_cast<double>(p.sum());
        return c.beta*(
            static_cast<double>(p[compartments::infected])/n_i +
            c.r_a*static_cast<double>(p[compartments::asymptomatic])/n_i +
            c.r_p*static_cast<double>(p[compartments::presymptomatic])/n_i);
      }

      patch::population_type
      patch::delta(
          const config_type& c, double dt,
          const population_type& commuting_in_population,
          size_t seed) const {
        population_type delta;
        double force = force_of_infection(c, commuting_in_population);
        std::mt19937_64 gen(seed);

        int x_1 = std::binomial_distribution<int>(
              _population[compartments::susceptible], force*dt)(gen);
        delta[compartments::susceptible] = -x_1;

        int x_inter = std::binomial_distribution<int>(
              _population[compartments::exposed], c.lambda_1*dt)(gen);
        delta[compartments::exposed] = x_1 - x_inter;

        int x_2 = std::binomial_distribution<int>(
            x_inter, 1.0-c.p_a)(gen);
        int x_4 = std::binomial_distribution<int>(
            _population[compartments::presymptomatic], c.lambda_2)(gen);
        delta[compartments::presymptomatic] = x_2 - x_4;

        int x_5 = std::binomial_distribution<int>(
            _population[compartments::infected], c.gamma)(gen);
        delta[compartments::infected] = x_4 - x_5;

        int x_6 = std::binomial_distribution<int>(
            _population[compartments::asymptomatic], c.gamma)(gen);
        delta[compartments::asymptomatic] = (x_inter - x_2) - x_6;

        delta[compartments::recovered] = x_5 + x_6;

        return delta;
      }

      void patch::apply_delta(const population_type& pop_delta) {
        _population += pop_delta;
      }
    }  // namespace fhi
  }  // namespace models
}  // namespace covid
