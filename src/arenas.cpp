#include <algorithm>
#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#pragma GCC diagnostic pop

#include "../include/covid/models/arenas/patch.hpp"

namespace covid {
  namespace models {
    namespace arenas {
      config::config(const std::string& model_config_filename) {
        csv::CSVReader reader(model_config_filename);
        for (auto&& row: reader) {
          std::string n = row[0].get<>();
          double v = row[1].get<double>();
          if (n == "beta_infected") beta_infected = v;
          else if (n == "beta_asymptomatic") beta_asymptomatic = v;
          else if (n == "kappa") kappa = v;
          else if (n == "xi") xi = v;
          else if (n == "p_y") p[age_groups::young] = v;
          else if (n == "p_m") p[age_groups::adults] = v;
          else if (n == "p_o") p[age_groups::elderly] = v;
          else if (n == "k_y") k[age_groups::young] = v;
          else if (n == "k_m") k[age_groups::adults] = v;
          else if (n == "k_o") k[age_groups::elderly] = v;
          else if (n == "gamma_y") gamma[age_groups::young] = v;
          else if (n == "gamma_m") gamma[age_groups::adults] = v;
          else if (n == "gamma_o") gamma[age_groups::elderly] = v;
          else if (n == "omega_y") omega[age_groups::young] = v;
          else if (n == "omega_m") omega[age_groups::adults] = v;
          else if (n == "omega_o") omega[age_groups::elderly] = v;
          else if (n == "reciprocal_eta_y")
            eta[age_groups::young] = 1.0/v;
          else if (n == "reciprocal_eta_m")
            eta[age_groups::adults] = 1.0/v;
          else if (n == "reciprocal_eta_o")
            eta[age_groups::elderly] = 1.0/v;
          else if (n == "reciprocal_alpha_y")
            alpha[age_groups::young] = 1.0/v;
          else if (n == "reciprocal_alpha_m")
            alpha[age_groups::adults] = 1.0/v;
          else if (n == "reciprocal_alpha_o")
            alpha[age_groups::elderly] = 1.0/v;
          else if (n == "reciprocal_mu_y")
            mu[age_groups::young] = 1.0/v;
          else if (n == "reciprocal_mu_m")
            mu[age_groups::adults] = 1.0/v;
          else if (n == "reciprocal_mu_o")
            mu[age_groups::elderly] = 1.0/v;
          else if (n == "reciprocal_psi_y")
            psi[age_groups::young] = 1.0/v;
          else if (n == "reciprocal_psi_m")
            psi[age_groups::adults] = 1.0/v;
          else if (n == "reciprocal_psi_o")
            psi[age_groups::elderly] = 1.0/v;
          else if (n == "reciprocal_chi_y")
            chi[age_groups::young] = 1.0/v;
          else if (n == "reciprocal_chi_m")
            chi[age_groups::adults] = 1.0/v;
          else if (n == "reciprocal_chi_o")
            chi[age_groups::elderly] = 1.0/v;
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

      double patch::prob_contracting(
          age_groups g,
          const config_type& c,
          const patch::population_type& in_population,
          const patch::population_type& out_population,
          double normalization_factor) const {
        double n_eff = 0.0;
        for (auto&& h: magic_enum::enum_values<age_groups>())
          n_eff += effective_population(h, c, in_population, out_population);

        double contacts =
          normalization_factor*c.k[g]*
          contact_function(n_eff/_area, c);

        double total_infected_exponents = 0.0;
        double total_asymptomatic_exponents = 0.0;
        for (auto&& h: magic_enum::enum_values<age_groups>()) {
          double in_infected =
            + _population[h][compartments::infected]
            - c.p[h]*out_population[h][compartments::infected]
            + c.p[h]*in_population[h][compartments::infected];
          total_infected_exponents += contacts*in_infected*
            c.contact[g][h]/
            effective_population(h, c, in_population, out_population);

          double in_asymptomatic =
            + _population[h][compartments::asymptomatic]
            - c.p[h]*out_population[h][compartments::asymptomatic]
            + c.p[h]*in_population[h][compartments::asymptomatic];
          total_asymptomatic_exponents += contacts*in_asymptomatic*
            c.contact[g][h]/
            effective_population(h, c, in_population, out_population);
        }

        return 1.0 -
          std::pow(1.0-c.beta_asymptomatic, total_asymptomatic_exponents)*
          std::pow(1.0-c.beta_infected, total_infected_exponents);
      }

      double patch::normalization_denominator(
          age_groups g,
          const config_type& c,
          const patch::population_type& in_population,
          const patch::population_type& out_population) const {
        double n_eff = 0.0;
        for (auto&& h: magic_enum::enum_values<age_groups>())
          n_eff += effective_population(h, c, in_population, out_population);

        return contact_function(n_eff/_area, c)*
          effective_population(g, c, in_population, out_population);
      }

      double patch::contact_function(double x, const config_type& c) const {
          return 1 + (1 - std::exp(-c.xi*x));
      }

      double patch::effective_population(
          age_groups g,
          const config_type& c,
          const patch::population_type& in_population,
          const patch::population_type& out_population) const {
        return (_population[g] +
            c.p[g]*(in_population[g] - out_population[g])).sum();
      }

      patch::patch(const population_type& population, double area)
        : _population(population), _area(area) {}

      const patch::population_type& patch::population() const {
        return _population;
      }

      double patch::area() const {
        return _area;
      }

      patch::population_type
      patch::delta(
          const config_type& c,
          double dt,
          const patch::population_type& in_population,
          const patch::population_type& out_population,
          age_array_type<double> normalization_factor,
          age_array_type<double> external_pi) const {
        population_type delta;
        for (auto&& g: magic_enum::enum_values<age_groups>()) {
          double r_ii =
            (_population[g] - out_population[g]).sum()/_population[g].sum();
          double pi = external_pi[g] + ((1.0 - c.p[g]) + c.p[g]*r_ii)*
            prob_contracting(g, c,
                in_population, out_population, normalization_factor[g]);

          if (pi < -1)
            std::cerr << "pi " << pi <<
              " external " << external_pi[g] << std::endl;

          delta[g][compartments::susceptible] =
            -pi*_population[g][compartments::susceptible];

          delta[g][compartments::exposed] =
            +pi*_population[g][compartments::susceptible]
            -c.eta[g]*_population[g][compartments::exposed];

          delta[g][compartments::asymptomatic] =
            +c.eta[g]*_population[g][compartments::exposed]
            -c.alpha[g]*_population[g][compartments::asymptomatic];

          delta[g][compartments::infected] =
            +c.alpha[g]*_population[g][compartments::asymptomatic]
            -c.mu[g]*_population[g][compartments::infected];

          delta[g][compartments::hospitalized] =
            +c.mu[g]*c.gamma[g]*_population[g][compartments::infected]
            -c.omega[g]*(c.psi[g]-c.chi[g])*
              _population[g][compartments::hospitalized]
            -c.chi[g]*_population[g][compartments::hospitalized];

          delta[g][compartments::dead] =
            +c.omega[g]*c.psi[g]*_population[g][compartments::hospitalized];

          delta[g][compartments::recovered] =
            +(1.0-c.omega[g])*c.chi[g]*
              _population[g][compartments::hospitalized]
            +(1.0-c.gamma[g])*c.mu[g]*
              _population[g][compartments::infected];
        }

        return delta*dt;
      }

      void patch::apply_delta(const population_type& pop_delta) {
        _population += pop_delta;
      }
    }  // namespace arenas
  }  // namespace models
}  // namespace covid
