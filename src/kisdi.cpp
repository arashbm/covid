#include <algorithm>
#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#pragma GCC diagnostic pop

#include "../include/covid/models/kisdi/patch.hpp"
#include "../include/covid/models/kisdi/config.hpp"

namespace covid {
  namespace models {
    namespace kisdi {
      config::config(const std::string& model_config_filename) {
        csv::CSVReader reader(model_config_filename);
        for (auto&& row: reader) {
          std::string n = row[0].get<>();
          double v = row[1].get<double>();
          if (n == "beta_infected") beta_infected = v;
          else if (n == "beta_asymptomatic") beta_asymptomatic = v;
          else if (n == "beta_presymptomatic") beta_presymptomatic = v;
          else if (n == "pi") pi = v;
          else if (n == "kappa") kappa = v;
          else if (n == "reciprocal_eta") eta = 1.0/v;
          else if (n == "reciprocal_alpha") alpha = 1.0/v;
          else if (n == "reciprocal_theta") theta = 1.0/v;
          else if (n == "reciprocal_nu") nu = 1.0/v;
          else if (n == "reciprocal_rho") rho = 1.0/v;
          else if (n == "reciprocal_chi") chi = 1.0/v;
          else if (n == "reciprocal_delta") delta = 1.0/v;
          else if (n == "c_yy")
            contact[age_groups::young][age_groups::young] = v;
          else if (n == "c_ym")
            contact[age_groups::young][age_groups::adults] = v;
          else if (n == "c_yo")
            contact[age_groups::young][age_groups::elderly] = v;
          else if (n == "c_my")
            contact[age_groups::adults][age_groups::young] = v;
          else if (n == "c_mm")
            contact[age_groups::adults][age_groups::adults] = v;
          else if (n == "c_mo")
            contact[age_groups::adults][age_groups::elderly] = v;
          else if (n == "c_oy")
            contact[age_groups::elderly][age_groups::young] = v;
          else if (n == "c_om")
            contact[age_groups::elderly][age_groups::adults] = v;
          else if (n == "c_oo")
            contact[age_groups::elderly][age_groups::elderly] = v;
        }
      }

      double
      patch::force_of_infecton(age_groups g, const config_type& c) const {
        double lambda = 0.0;
        contact_matrix_type contacts = contact_function(c);
        for (auto&& h: all_age_groups) {
          double x_h = contact_making_population(h, c);
          lambda += contacts[g][h]*
            (c.beta_asymptomatic*_population[h][compartments::asymptomatic] +
            c.beta_presymptomatic*_population[h][compartments::presymptomatic] +
            c.beta_infected*c.kappa*_population[h][compartments::infected])/x_h;
        }
        return lambda;
      }

      contact_matrix_type patch::contact_function(
          const config_type& c) const {
        contact_matrix_type contact;
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
      patch::contact_making_population(
          age_groups g, const config_type& c) const {
        return _population[g][compartments::susceptible]
          + _population[g][compartments::exposed]
          + _population[g][compartments::presymptomatic]
          + _population[g][compartments::asymptomatic]
          + _population[g][compartments::recovered]
          + c.kappa*_population[g][compartments::infected];
      }

      patch::patch(const population_type& population)
        : _population(population) {}

      const patch::population_type& patch::population() const {
        return _population;
      }

      patch::population_type
      patch::delta(const config_type& c, double dt) const {
        population_type delta;
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

        return delta*dt;
      }

      void patch::apply_delta(const population_type& pop_delta) {
        _population += pop_delta;
      }
    }  // namespace kisdi
  }  // namespace models
}  // namespace covid
