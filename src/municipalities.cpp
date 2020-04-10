#include <unordered_map>
#include <string>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#pragma GCC diagnostic pop

#include "../include/covid/kisdi/metapop.hpp"
#include "../include/covid/kisdi/config.hpp"
#include "../include/covid/kisdi/patch.hpp"

std::unordered_map<std::string, covid::kisdi::patch>
create_patches(std::string population_filename) {
  csv::CSVReader reader(population_filename);

  // FIXME: I would've used header instead of numerical indecies
  std::unordered_map<covid::kisdi::age_groups, std::vector<size_t>>
    age_group_columns = {
      {covid::kisdi::age_groups::young,   {2, 3, 4, 5, 6, 7, 8}},
      {covid::kisdi::age_groups::adults,  {9, 10, 11, 12, 13, 14, 15, 16}},
      {covid::kisdi::age_groups::elderly, {17, 18, 19, 20, 21, 22}}};

  covid::kisdi::patch::PopulationMatrixType pop;

  // FIXME: Persumably we should read this from a source rather than guessing
  double presymptomatic_p = 4000/5.51e6,
         infected_p = 2000/5.51e6,
         asymptomatic_p = 4000/5.51e6,
         exposed_p = 3000/5.51e6,
         hospitalized_p = 200/5.51e6,
         dead_p = 34/5.51e6,
         recovered_p = 300/5.51e6;
  double susceptible_p = 1.0 - presymptomatic_p - infected_p - exposed_p
    - asymptomatic_p - dead_p - hospitalized_p - recovered_p;

  std::unordered_map<std::string, covid::kisdi::patch> patches;
  for (auto&& row: reader) {
    for (auto&& g: covid::kisdi::all_age_groups) {
      double total_pop_g = 0;
      for (size_t c: age_group_columns[g])
        total_pop_g += row[c].get<double>();
      pop[g][covid::kisdi::compartments::susceptible] =
        total_pop_g*susceptible_p;
      pop[g][covid::kisdi::compartments::exposed] =
        total_pop_g*exposed_p;
      pop[g][covid::kisdi::compartments::asymptomatic] =
        total_pop_g*asymptomatic_p;
      pop[g][covid::kisdi::compartments::presymptomatic] =
        total_pop_g*presymptomatic_p;
      pop[g][covid::kisdi::compartments::infected] =
        total_pop_g*infected_p;
      pop[g][covid::kisdi::compartments::hospitalized] =
        total_pop_g*hospitalized_p;
      pop[g][covid::kisdi::compartments::dead] =
        total_pop_g*dead_p;
      pop[g][covid::kisdi::compartments::recovered] =
        total_pop_g*recovered_p;
    }
    patches.try_emplace(row[0].get<>(), pop);
  }

  std::cerr << "population of espoo is " <<
    patches.at("Espoo").population().sum().sum() << std::endl;

  return patches;
}


// TODO: should create proper mobility class
using mobility_list = std::unordered_map<std::string,
      std::vector<std::pair<std::string, double>>>;

std::pair<mobility_list, mobility_list>
create_mobility_lists(std::string population_filename) {
  csv::CSVReader reader(population_filename);

  mobility_list in_mobility, out_mobility;

  double total_trips_per_day = 5.5e5;

  std::unordered_map<std::string, double> pop;
  for (auto&& row: reader)
    pop.try_emplace(row[0].get<>(), row[1].get<double>());

  double normalization_factor = 0.0;
  for (auto &[i, pop_i]: pop)
    for (auto &[j, pop_j]: pop)
      normalization_factor += pop_i*pop_j;

  for (auto &[i, pop_i]: pop) {
    for (auto &[j, pop_j]: pop) {
      double trips = total_trips_per_day*pop_i*pop_j/normalization_factor;
      if (trips > 100) {
        in_mobility[i].emplace_back(j, trips);
        out_mobility[j].emplace_back(i, trips);
      }
    }
  }

  return {in_mobility, out_mobility};
}

// FIXME: Persumably we should read this from a file
covid::kisdi::config create_config() {
  covid::kisdi::config conf;
  conf.pi = 0.60;
  conf.eta = 1.0/3.0;
  conf.theta = 1/8.0;
  conf.alpha = 1/2.0;
  conf.nu = 1/2.86;
  conf.rho = 1.0/5.0;
  conf.chi = 1.0/10.0;
  conf.delta = 1.0/7.0;
  conf.beta_infected = 0.06;
  conf.beta_asymptomatic = 0.06;
  conf.beta_presymptomatic = 0.06;
  conf.kappa = 0.2;
  conf.contact = covid::kisdi::ContactMatrixType({{
    {0.5, 0.4, 0.1},
    {0.3, 0.6, 0.1},
    {0.2, 0.6, 0.2}}});

  return conf;
}

void report_totals(double t,
    const std::unordered_map<std::string, covid::kisdi::patch>& patches) {
  covid::categorical_array<
    double, covid::kisdi::compartments, covid::kisdi::all_compartments.size()>
    totals;
  for (auto& p: patches)
    totals += p.second.population().sum();

  std::cout << t << " " << totals.sum();
  for (auto&& c: covid::kisdi::all_compartments)
    std::cout << " " << totals[c];
  std::cout << "\n";
}

int main(/* int argc , char* argv[]*/) {
  std::unordered_map<std::string, covid::kisdi::patch>
    patches = create_patches("data/municipalities.csv");

  std::cout << "t total susceptible exposed asymptomatic presymptomatic "
    "infected hospitalized dead recovered\n";
  report_totals(0.0, patches);

  auto [in_mob, out_mob] = create_mobility_lists("data/municipalities.csv");

  covid::kisdi::config conf = create_config();

  int dt = 1, t_max = 24*100;
  int report_dt = 24, next_report = 24;

  for (int t = dt; t < t_max; t += dt) {
    std::vector<
      std::pair<std::string, covid::kisdi::patch::PopulationMatrixType>>
      deltas;

    for (auto &[i, patch_i]: patches) {
      auto delta = patch_i.delta_population(conf);

      for (auto &[j, trips_per_day]: in_mob[i])
        delta += trips_per_day*patches.at(j).population();

      for (auto &[j, trips_per_day]: out_mob[i])
        delta -= trips_per_day*patches.at(j).population();

      deltas.emplace_back(i, delta);
    }

    for (auto& [i, delta]: deltas)
      patches.at(i).apply_delta(delta*(static_cast<double>(dt)/24.0));

    if (t == next_report) {
      next_report += report_dt;
      report_totals(static_cast<double>(t)/24.0, patches);
    }
  }
}
