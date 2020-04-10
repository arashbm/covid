#ifndef INCLUDE_COVID_KISDI_CONFIG_HPP_
#define INCLUDE_COVID_KISDI_CONFIG_HPP_

#include "metapop.hpp"

namespace covid {
  namespace kisdi {
    struct config {
      double pi, eta, theta, nu, rho, chi, delta, alpha;

      double beta_infected, beta_asymptomatic, beta_presymptomatic, kappa;

      ContactMatrixType contact = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

      config() = default;
    };
  }  // namespace kisdi
}  // namespace covid

#endif  // INCLUDE_COVID_KISDI_CONFIG_HPP_
