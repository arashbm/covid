#ifndef INCLUDE_COVID_MODELS_ARENAS_CONFIG_HPP_
#define INCLUDE_COVID_MODELS_ARENAS_CONFIG_HPP_

#include <string>

#include "metapop.hpp"

namespace covid {
  namespace models {
    namespace arenas {
      struct config {
        age_array_type<double> eta, alpha, mu, psi, chi; //rates

        age_array_type<double> omega, gamma, p, k;

        double beta_infected, beta_asymptomatic, kappa, xi;

        contact_matrix_type contact = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        explicit config(const std::string& model_config_filename);
        config() = default;
      };
    }  // namespace arenas
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_ARENAS_CONFIG_HPP_
