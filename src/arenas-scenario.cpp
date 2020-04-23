#include <unordered_map>
#include <string>
#include <iostream>
#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#pragma GCC diagnostic pop
#include "args.hxx"

#include "../include/covid/models/arenas/metapop.hpp"
#include "../include/covid/models/arenas/patch.hpp"
#include "../include/covid/mobility/gravity.hpp"
namespace arenas = covid::models::arenas;


std::unordered_map<std::string, arenas::patch>
create_patches(
    const std::string& population_filename,
    const std::string& initial_condition_filename) {

  std::unordered_map<std::string, arenas::patch::population_type> pops;
  std::unordered_map<std::string, double> surface_area;

  std::unordered_map<arenas::age_groups, std::string>
    age_group_columns = {
      {arenas::age_groups::young,   "Young"},
      {arenas::age_groups::adults,  "Adults"},
      {arenas::age_groups::elderly, "Elderly"}};

  std::unordered_map<arenas::compartments, std::string>
    compartment_columns = {
      {arenas::compartments::exposed,        "Exposed"},
      {arenas::compartments::asymptomatic,   "Asymptomatic"},
      {arenas::compartments::infected,       "Infected"},
      {arenas::compartments::hospitalized,   "Hospitalized"},
      {arenas::compartments::dead,           "Recovered"},
    };

  csv::CSVReader population_reader(population_filename);
  for (auto&& row: population_reader) {
    for (auto&& g: arenas::all_age_groups) {
      std::string id = row["Area"].get<>();
      pops[id][g][arenas::compartments::susceptible] =
        row[age_group_columns[g]].get<double>();
      surface_area[id] = row["Populated squares"].get<double>();
    }
  }

  csv::CSVReader initial_reader(initial_condition_filename);
  for (auto&& row: initial_reader) {
    for (auto&& [g, g_name]: age_group_columns) {
      for (auto&& [c, c_name]: compartment_columns) {
        std::string col_name = c_name + " " + g_name;
        if (initial_reader.index_of(col_name) != csv::CSV_NOT_FOUND) {
          double p = row[col_name].get<double>();
          std::string id = row["Area"].get<>();
          pops[id][g][c] = p;
          pops[id][g][arenas::compartments::susceptible] -= p;
        }
      }
    }
  }

  std::unordered_map<std::string, arenas::patch> patches;
  for (auto &[i, pop]: pops)
    patches.try_emplace(i, pop, surface_area[i]);

  return patches;
}

void report_totals(double t,
    const std::unordered_map<std::string, arenas::patch>& patches) {
  covid::categorical_array<
    double, arenas::compartments, arenas::all_compartments.size()>
    totals;
  for (auto& p: patches)
    totals += p.second.population().sum();

  std::cout << t << " " << totals.sum();
  for (auto&& c: arenas::all_compartments)
    std::cout << " " << totals[c];
  std::cout << "\n";
}

int main(int argc , char* argv[]) {
  args::ArgumentParser parser(
      "This is a test program.", "This goes after the options.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});


  args::ValueFlag<size_t> simulation_days(parser,
      "simulation_days",
      "simulated time (unsigned integer) in days",
      {'t', "simulation_days"}, 100ul);
  args::ValueFlag<size_t> seed(parser,
      "seed",
      "seed (unsigned integer) for random number generator",
      {'s', "seed"}, 0ul);

  args::Group inputs(parser, "Inputs (All mandatory)",
      args::Group::Validators::All);
  args::ValueFlag<std::string> population_filename(inputs,
      "population",
      "population file (CSV)",
      {'p', "population"});
  args::ValueFlag<std::string> initial_condition_filename(inputs,
      "initial-condition",
      "initial number of exposed people in each municipality (CSV)",
      {'i', "initial-condition"});
  args::ValueFlag<std::string> model_config_filename(inputs,
      "model-configuration",
      "parameters for model (CSV)",
      {'c', "model-configuration"});

  try {
    parser.ParseCLI(argc, argv);
  } catch (const args::Help&) {
    std::cout << parser;
    return 0;
  } catch (const args::ParseError& e) {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  } catch (const args::ValidationError& e) {
    std::cerr << "Given command: ";
    for (int i = 0; i < argc; i++)
      std::cerr << argv[i] << " ";
    std::cerr << std::endl;

    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }

  std::unordered_map<std::string, arenas::patch>
    patches = create_patches(
        args::get(population_filename),
        args::get(initial_condition_filename));

  covid::mobility::gravity mobility(args::get(population_filename));

  arenas::patch::config_type conf(args::get(model_config_filename));

  size_t dt = 1, t_max = args::get(simulation_days)*24;
  size_t report_dt = 24, next_report = 0;

  std::cout << "t total susceptible exposed asymptomatic infected hospitalized "
    "dead recovered\n";

  std::unordered_map<std::string,
    std::optional<covid::mobility::gravity::hint_type>> hints_from, hints_to;

  for (size_t t = 0; t < t_max; t += dt) {
    double time_days = static_cast<double>(t)/24.0,
           dt_days = static_cast<double>(dt)/24.0;

    if (t == next_report) {
      next_report += report_dt;
      report_totals(time_days, patches);
    }

    std::vector<std::pair<std::string, arenas::patch::population_type>>
      deltas;

    for (auto &[i, patch_i]: patches) {
      arenas::patch::population_type in_population;
      auto [mobility_to, hint_to] = mobility.trips_to(
          i, time_days, dt_days, hints_to[i]);
      for (auto &[j, trips_per_dt]: mobility_to)
        in_population += trips_per_dt*
          patches.at(j).population()/patches.at(j).population().sum().sum();
      hints_to[i] = hint_to;

      arenas::patch::population_type out_population;
      auto [mobility_from, hint_from] = mobility.trips_from(
          i, time_days, dt_days, hints_from[i]);
      for (auto &[j, trips_per_dt]: mobility_from)
        out_population += trips_per_dt*
          patches.at(j).population()/patches.at(j).population().sum().sum();
      hints_from[i] = hint_from;

      arenas::age_array_type<double> normalization_factor = {1.0, 1.0, 1.0};
      for (auto&& g: arenas::all_age_groups) {
        double total_pop = 0.0;
        double total_denominator = 0.0;
        for (auto &[i, patch_i]: patches) {
          total_pop +=
            patch_i.population()[g].sum();
          total_denominator +=
            patch_i.normalization_denominator(g, conf,
                in_population, out_population);
        }

        normalization_factor[g] = total_pop/total_denominator;
      }

      arenas::age_array_type<double> external_pi = {0, 0, 0};
      for (auto &[j, trips_per_dt]: mobility_from)
        for (auto&& g: arenas::all_age_groups)
          external_pi[g] +=
            conf.p[g]*patches.at(j).prob_contracting(g, conf,
                in_population, out_population, normalization_factor[g]) *
            trips_per_dt/patch_i.population().sum().sum();

      auto delta = patch_i.delta(conf, dt_days,
          in_population, out_population, normalization_factor, external_pi);
      deltas.emplace_back(i, delta);
    }

    for (auto& [i, delta]: deltas)
      patches.at(i).apply_delta(delta);
  }
}
