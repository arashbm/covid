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

      constexpr std::array<compartments, 8> all_compartments = {
        compartments::susceptible,
        compartments::exposed,
        compartments::asymptomatic,
        compartments::presymptomatic,
        compartments::infected,
        compartments::hospitalized,
        compartments::dead,
        compartments::recovered,
      };

      enum class age_groups : size_t {
        young,    // (Y) age <= 25
        adults,   // (M) 26 <= age <= 65
        elderly,  // (O) 66 <= age
      };

      constexpr std::array<age_groups, 3> all_age_groups = {
        age_groups::young,
        age_groups::adults,
        age_groups::elderly,
      };


      template <typename T>
      using age_array_type =
        categorical_array<T, age_groups, all_age_groups.size()>;

      template <typename T>
      using compartment_array_type =
        categorical_array<T, compartments, all_compartments.size()>;

      using contact_matrix_type = age_array_type<age_array_type<double>>;
    }  // namespace kisdi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_KISDI_METAPOP_HPP_
