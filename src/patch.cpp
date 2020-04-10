#include <algorithm>

#include "../include/covid/kisdi/patch.hpp"

namespace covid {
  namespace kisdi {
    double patch::force_of_infecton(age_groups g, const config& c) const {
      double lambda = 0.0;
      ContactMatrixType contacts = contact_function(c);
      for (auto&& h: all_age_groups) {
        double x_h = contact_making_population(h, c);
        lambda += contacts[g][h]*
          (c.beta_asymptomatic*_population[h][compartments::asymptomatic] +
          c.beta_presymptomatic*_population[h][compartments::presymptomatic] +
          c.beta_infected*c.kappa*_population[h][compartments::infected])/x_h;
      }
      return lambda;
    }

    ContactMatrixType patch::contact_function(const config& c) const {
      ContactMatrixType contact;
      for (auto&& g: all_age_groups) {
        double x_g = contact_making_population(g, c);
        for (auto&& h: all_age_groups) {
          double x_h = contact_making_population(h, c);
          contact[g][h] = std::min(
              c.contact[g][h]*x_g,
              c.contact[h][g]*x_h)/x_g;
          }
      }
      return contact;
    }

    double
    patch::contact_making_population(age_groups g, const config& c) const {
      return _population[g][compartments::susceptible]
        + _population[g][compartments::exposed]
        + _population[g][compartments::presymptomatic]
        + _population[g][compartments::asymptomatic]
        + _population[g][compartments::recovered]
        + c.kappa*_population[g][compartments::infected];
    }

    patch::patch(const PopulationMatrixType& population) {
      _population = population;
    }

    const patch::PopulationMatrixType& patch::population() const {
      return _population;
    }

    patch::PopulationMatrixType patch::delta_population(const config& c) const {
      PopulationMatrixType delta;
      for (auto&& g: all_age_groups) {
        double lambda = force_of_infecton(g, c);

        delta[g][compartments::susceptible] =
          -lambda*_population[g][compartments::susceptible];

        delta[g][compartments::exposed] =
          +lambda*_population[g][compartments::susceptible]
          -c.eta*_population[g][compartments::exposed];

        delta[g][compartments::asymptomatic] =
          +(1.0-c.pi)*c.eta*_population[g][compartments::exposed]
          -c.rho*_population[g][compartments::asymptomatic];

        delta[g][compartments::presymptomatic] =
          +c.pi*c.eta*_population[g][compartments::exposed]
          -c.alpha*_population[g][compartments::presymptomatic];

        delta[g][compartments::infected] =
          +c.alpha*_population[g][compartments::presymptomatic]
          -(c.theta+c.nu)*_population[g][compartments::infected];

        delta[g][compartments::hospitalized] =
          +c.theta*_population[g][compartments::infected]
          -(c.chi+c.delta)*_population[g][compartments::hospitalized];

        delta[g][compartments::dead] =
          +c.delta*_population[g][compartments::hospitalized];

        delta[g][compartments::recovered] =
          +c.rho*_population[g][compartments::asymptomatic]
          +c.nu*_population[g][compartments::infected]
          +c.chi*_population[g][compartments::hospitalized];
      }

      return delta;
    }

    void patch::apply_delta(const PopulationMatrixType& pop_delta) {
      _population += pop_delta;
    }
  }  // namespace kisdi
}  // namespace covid
