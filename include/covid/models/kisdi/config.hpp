#ifndef INCLUDE_COVID_MODELS_KISDI_CONFIG_HPP_
#define INCLUDE_COVID_MODELS_KISDI_CONFIG_HPP_

#include <string>

#include "metapop.hpp"

namespace covid {
  namespace models {
    namespace kisdi {
      struct config {
        double pi, eta, theta, nu, rho, chi, delta, alpha;

        double beta_infected, beta_asymptomatic, beta_presymptomatic, kappa;

        contact_matrix_type contact = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

        config() = default;
        explicit config(const std::string& model_config_filename);
      };
    }  // namespace kisdi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_KISDI_CONFIG_HPP_
