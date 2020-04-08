#ifndef INCLUDE_COVID_PATCH_HPP_
#define INCLUDE_COVID_PATCH_HPP_

#include "metapop.hpp"
#include "config.hpp"
#include "categorical_array.hpp"

namespace covid {
  class patch {
  public:
    using PopulationMatrixType = AgeArrayType<
      categorical_array<double, compartments, all_compartments.size()>>;

    explicit patch(const PopulationMatrixType& population);

    const PopulationMatrixType& population() const;
    PopulationMatrixType delta_population(const config& c) const;
    void apply_delta(const PopulationMatrixType& pop_delta);
  private:
    PopulationMatrixType _population;  // population[age_group][compartment]

    double force_of_infecton(age_groups g, const config& c) const;
    ContactMatrixType contact_function(const config& c) const;
    double contact_making_population(age_groups g, const config& c) const;
  };
}  // namespace covid

#endif  // INCLUDE_COVID_PATCH_HPP_
