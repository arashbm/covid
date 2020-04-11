#ifndef INCLUDE_COVID_MODELS_KISDI_PATCH_HPP_
#define INCLUDE_COVID_MODELS_KISDI_PATCH_HPP_

#include "../../categorical_array.hpp"
#include "metapop.hpp"
#include "config.hpp"

namespace covid {
  namespace models {
    namespace kisdi {
      class patch {
      public:
        using population_type = age_array_type<
          categorical_array<double, compartments, all_compartments.size()>>;

        using config_type = config;

        explicit patch(const population_type& population);

        const population_type& population() const;
        population_type delta(const config_type& c) const;
        void apply_delta(const population_type& pop_delta);

      private:
        population_type _population;  // population[age_group][compartment]

        double
        force_of_infecton(age_groups g, const config_type& c) const;

        contact_matrix_type
        contact_function(const config_type& c) const;

        double
        contact_making_population(age_groups g, const config_type& c) const;
      };
    }  // namespace kisdi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_KISDI_PATCH_HPP_
