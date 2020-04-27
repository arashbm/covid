#include <unordered_map>
#include <string>
#include <iostream>
#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#include "magic_enum.hpp"
#pragma GCC diagnostic pop
#include "args.hxx"

#include "../include/covid/models/kisdi/metapop.hpp"
#include "../include/covid/models/kisdi/patch.hpp"
#include "../include/covid/mobility/gravity.hpp"
#include "report.tpp"
namespace kisdi = covid::models::kisdi;


std::unordered_map<std::string, kisdi::patch>
create_patches(
    const std::string& population_filename,
    const std::string& initial_condition_filename) {

  std::unordered_map<std::string, kisdi::patch::population_type> pops;

  std::unordered_map<kisdi::age_groups, std::string>
    age_group_columns = {
      {kisdi::age_groups::young,   "Young"},
      {kisdi::age_groups::adults,  "Adults"},
      {kisdi::age_groups::elderly, "Elderly"}};

  std::unordered_map<kisdi::compartments, std::string>
    compartment_columns = {
      {kisdi::compartments::exposed,        "Exposed"},
      {kisdi::compartments::asymptomatic,   "Asymptomatic"},
      {kisdi::compartments::presymptomatic, "Presymptomatic"},
      {kisdi::compartments::infected,       "Infected"},
      {kisdi::compartments::hospitalized,   "Hospitalized"},
      {kisdi::compartments::dead,           "Dead"},
      {kisdi::compartments::recovered,      "Recovered"},
    };

  csv::CSVReader population_reader(population_filename);
  for (auto&& row: population_reader)
    for (auto&& g: magic_enum::enum_values<kisdi::age_groups>())
      pops[row["Area"].get<>()][g][kisdi::compartments::susceptible] =
        row[age_group_columns[g]].get<double>();

  csv::CSVReader initial_reader(initial_condition_filename);
  for (auto&& row: initial_reader) {
    for (auto&& [g, g_name]: age_group_columns) {
      for (auto&& [c, c_name]: compartment_columns) {
        std::string col_name = c_name + " " + g_name;
        if (initial_reader.index_of(col_name) != csv::CSV_NOT_FOUND)
          pops[row["Area"].get<>()][g][c] = row[col_name].get<double>();
      }
    }
  }

  std::unordered_map<std::string, kisdi::patch> patches;
  for (auto &[i, pop]: pops)
    patches.try_emplace(i, pop);

  return patches;
}

kisdi::patch::population_type
mobile_population(kisdi::patch::population_type pop) {
  for (auto&& g: magic_enum::enum_values<kisdi::age_groups>())
    for (auto&& c: magic_enum::enum_values<kisdi::compartments>())
      if (c != kisdi::compartments::susceptible &&
          c != kisdi::compartments::exposed &&
          c != kisdi::compartments::asymptomatic &&
          c != kisdi::compartments::presymptomatic)
        pop[g][c] = 0.0;
  return pop;
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

  std::unordered_map<std::string, kisdi::patch>
    patches = create_patches(
        args::get(population_filename),
        args::get(initial_condition_filename));

  covid::mobility::gravity mobility(args::get(population_filename));

  kisdi::patch::config_type conf(args::get(model_config_filename));

  std::vector<std::string> notable_patches = {};  // TODO

  auto compartments_array = magic_enum::enum_values<kisdi::compartments>();
  std::vector<kisdi::compartments> reported_compartments(
      compartments_array.begin(), compartments_array.end());

  size_t dt = 1, t_max = args::get(simulation_days)*24;
  size_t report_dt = 24, next_report = 0;

  covid::report::population_report_header(
      notable_patches, reported_compartments,
      std::cout);

  std::unordered_map<std::string,
    std::optional<covid::mobility::gravity::hint_type>> hints_from, hints_to;

  for (size_t t = 0; t < t_max; t += dt) {
    double time_days = static_cast<double>(t)/24.0,
           dt_days = static_cast<double>(dt)/24.0;

    if (t == next_report) {
      next_report += report_dt;
      covid::report::age_grouped_population_report(
          time_days, patches, notable_patches, reported_compartments,
          std::cout);
    }

    std::vector<std::pair<std::string, kisdi::patch::population_type>>
      deltas;

    for (auto &[i, patch_i]: patches) {
      auto delta = patch_i.delta(conf, dt_days);

      auto [mobility_to, hint_to] = mobility.trips_to(
          i, time_days, dt_days, hints_to[i]);
      for (auto &[j, trips_per_dt]: mobility_to)
        delta += mobile_population(trips_per_dt*
            patches.at(j).population()/patches.at(j).population().sum().sum());
      hints_to[i] = hint_to;

      auto [mobility_from, hint_from] = mobility.trips_from(
          i, time_days, dt_days, hints_from[i]);
      for (auto &[j, trips_per_dt]: mobility_from)
        delta -= mobile_population(trips_per_dt*
            patches.at(j).population()/patches.at(j).population().sum().sum());
      hints_from[i] = hint_from;

      deltas.emplace_back(i, delta);
    }

    for (auto& [i, delta]: deltas)
      patches.at(i).apply_delta(delta);
  }
}
