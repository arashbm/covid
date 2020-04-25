#ifndef INCLUDE_COVID_MODELS_KISDI_METAPOP_HPP_
#define INCLUDE_COVID_MODELS_KISDI_METAPOP_HPP_

#include <array>

#include "../../categorical_array.hpp"

namespace covid {
  namespace models {
    namespace kisdi {
      enum class compartments : size_t {
        susceptible,     // (S)
        exposed,         // (E)
        asymptomatic,    // (A)
        presymptomatic,  // (P)
        infected,        // (I)
        hospitalized,    // (H) in ICU
        dead,            // (D)
        recovered,       // (R)
      };

      enum class age_groups : size_t {
        young,    // (Y) age <= 25
        adults,   // (M) 26 <= age <= 65
        elderly,  // (O) 66 <= age
      };

      template <typename T>
      using age_array_type =
        categorical_array<T, age_groups>;

      template <typename T>
      using compartment_array_type =
        categorical_array<T, compartments>;

      using contact_matrix_type = age_array_type<age_array_type<double>>;
    }  // namespace kisdi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_KISDI_METAPOP_HPP_
