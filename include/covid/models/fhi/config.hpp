#ifndef INCLUDE_COVID_MODELS_FHI_CONFIG_HPP_
#define INCLUDE_COVID_MODELS_FHI_CONFIG_HPP_

#include <string>

#include "metapop.hpp"

namespace covid {
  namespace models {
    namespace fhi {
      struct config {
        double lambda_1, lambda_2, gamma;

        double beta, p_a, r_a, r_p;

        explicit config(const std::string& model_config_filename);
        config() = default;
      };
    }  // namespace fhi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_FHI_CONFIG_HPP_
