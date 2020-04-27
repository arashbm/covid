#ifndef INCLUDE_COVID_MODELS_FHI_METAPOP_HPP_
#define INCLUDE_COVID_MODELS_FHI_METAPOP_HPP_

#include <array>

#include "../../categorical_array.hpp"

namespace covid {
  namespace models {
    namespace fhi {
      enum class compartments : size_t {
        susceptible,     // (S)
        exposed,         // (E1)
        presymptomatic,  // (E2)
        infected,        // (I)
        asymptomatic,    // (Ia)
        recovered,       // (R)
      };

      template <typename T>
      using compartment_array_type = categorical_array<T, compartments>;
    }  // namespace fhi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_FHI_METAPOP_HPP_
