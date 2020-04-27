#ifndef INCLUDE_COVID_MODELS_FHI_PATCH_HPP_
#define INCLUDE_COVID_MODELS_FHI_PATCH_HPP_

#include "../../categorical_array.hpp"
#include "metapop.hpp"
#include "config.hpp"

namespace covid {
  namespace models {
    namespace fhi {
      class patch {
      public:
        using population_type = compartment_array_type<int>;
        using config_type = config;

        explicit patch(const population_type& population);

        const population_type& population() const;

        population_type delta(
          const config_type& c,
          double dt,
          const population_type& commuting_in_population,
          size_t seed) const;

        void apply_delta(const population_type& pop_delta);
      private:
        population_type _population;  // population[compartment]

        double force_of_infection(
            const config_type& c,
            const population_type& commuting_in_population) const;
      };
    }  // namespace fhi
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_FHI_PATCH_HPP_
