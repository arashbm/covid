#ifndef INCLUDE_COVID_MODELS_ARENAS_PATCH_HPP_
#define INCLUDE_COVID_MODELS_ARENAS_PATCH_HPP_

#include "../../categorical_array.hpp"
#include "metapop.hpp"
#include "config.hpp"

namespace covid {
  namespace models {
    namespace arenas {
      class patch {
      public:
        using population_type = age_array_type<compartment_array_type<double>>;
        using config_type = config;

        explicit patch(const population_type& population, double area);

        const population_type& population() const;
        double area() const;

        population_type delta(
          const config_type& c,
          double dt,
          const population_type& in_population,
          const population_type& out_population,
          age_array_type<double> normalization_factor,
          age_array_type<double> external_pi) const;

        void apply_delta(const population_type& pop_delta);

        double prob_contracting(
            age_groups g,
            const config_type& c,
            const population_type& in_population,
            const population_type& out_population,
            double normalization_factor) const;

        double normalization_denominator(
            age_groups g,
            const config_type& c,
            const population_type& in_population,
            const population_type& out_population) const;
      private:
        population_type _population;  // population[age_group][compartment]
        double _area;

        double contact_function(double x, const config_type& c) const;

        double effective_population(
          age_groups g,
          const config_type& c,
          const population_type& in_population,
          const population_type& out_population) const;
      };
    }  // namespace arenas
  }  // namespace models
}  // namespace covid

#endif  // INCLUDE_COVID_MODELS_ARENAS_PATCH_HPP_
