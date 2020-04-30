#include <unordered_map>
#include <string>
#include <iostream>
#include <numeric>
#include <random>
#include <cmath>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "csv.hpp"
#include "magic_enum.hpp"
#pragma GCC diagnostic pop
#include "args.hxx"

#include "../include/covid/models/fhi/metapop.hpp"
#include "../include/covid/models/fhi/patch.hpp"
#include "../include/covid/mobility/traffic.hpp"
#include "../include/covid/hashing.hpp"
#include "report.tpp"
namespace fhi = covid::models::fhi;


std::unordered_map<std::string, fhi::patch>
create_patches(
    const std::string& population_filename,
    const std::string& initial_condition_filename) {

  std::unordered_map<std::string, fhi::patch::population_type> pops;

  std::unordered_map<fhi::compartments, std::string>
    compartment_columns = {
      {fhi::compartments::exposed,        "Exposed"},
      {fhi::compartments::asymptomatic,   "Asymptomatic"},
      {fhi::compartments::presymptomatic, "Presymptomatic"},
      {fhi::compartments::infected,       "Infected"},
      {fhi::compartments::recovered,      "Recovered"},
    };

  csv::CSVReader population_reader(population_filename);
  for (auto&& row: population_reader)
    pops[row["Area"].get<>()][fhi::compartments::susceptible] =
      row["Total population"].get<int>();

  csv::CSVReader initial_reader(initial_condition_filename);
  for (auto&& row: initial_reader)
    for (auto&& [c, c_name]: compartment_columns)
      if (initial_reader.index_of(compartment_columns[c]) != csv::CSV_NOT_FOUND)
        pops[row["Area"].get<>()][c] =
          row[compartment_columns[c]].get<int>();

  std::unordered_map<std::string, fhi::patch> patches;
  for (auto &[i, pop]: pops)
    patches.try_emplace(i, pop);

  return patches;
}

fhi::patch::population_type
commuting_population(
    const fhi::patch::population_type& pop,
    size_t commuters,
    size_t seed) {
  fhi::patch::population_type samples;
  std::vector<int> weights;

  for (size_t i = 0; i < magic_enum::enum_count<fhi::compartments>(); i++)
    weights.push_back(
        pop[magic_enum::enum_value<fhi::compartments>(i)]);

  // assumed infected (symptomatic) people don't travel
  weights[magic_enum::enum_index<fhi::compartments>(
      fhi::compartments::infected).value()] = 0;

  /* if (std::accumulate(weights.begin(), weights.end(), 0ul) > commuters) */
  /*   throw std::runtime_error("Commuters larger than mobile population " */
  /*       "of the home patch"); */

  commuters = std::min(commuters,
      std::accumulate(weights.begin(), weights.end(), 0ul));

  std::mt19937_64 gen(seed);
  for (size_t i = 0; i < commuters; i++) {
    size_t comp = std::discrete_distribution<size_t>(
        weights.begin(), weights.end())(gen);
    samples[magic_enum::enum_value<fhi::compartments>(comp)] += 1;
    weights[comp] -= 1;
  }

  return samples;
}

int main(int argc , char* argv[]) {
  args::ArgumentParser parser(
      "This is a test program.", "This goes after the options.");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});


  args::ValueFlag<size_t> simulation_days(parser,
      "simulation_days",
      "simulated time (unsigned integer) in days",
      {'t', "simulation_days"}, 100ul);
  args::ValueFlag<size_t> random_seed(parser,
      "seed",
      "seed (unsigned integer) for random number generator",
      {'s', "seed"}, 0ul);

  args::Group inputs(parser, "Inputs (All mandatory)",
      args::Group::Validators::All);
  args::ValueFlag<std::string> population_filename(inputs,
      "population",
      "population file (CSV)",
      {'p', "population"});
  args::ValueFlag<std::string> mobility_filename(inputs,
      "mobility",
      "mobility file (CSV)",
      {'m', "mobility"});
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

  std::unordered_map<std::string, fhi::patch>
    patches = create_patches(
        args::get(population_filename),
        args::get(initial_condition_filename));

  covid::mobility::traffic mobility(
      args::get(mobility_filename),
      args::get(population_filename));

  fhi::patch::config_type conf(args::get(model_config_filename));

  size_t seed = args::get(random_seed);

  std::vector<std::string> notable_patches = {};  // TODO

  auto compartments_array = magic_enum::enum_values<fhi::compartments>();
  std::vector<fhi::compartments> reported_compartments(
      compartments_array.begin(), compartments_array.end());

  size_t dt = 1, t_max = args::get(simulation_days)*24;

  covid::report::population_report_header(
      notable_patches, reported_compartments,
      std::cout);

  std::unordered_map<std::string,
    std::optional<covid::mobility::traffic::hint_type>> hints_from;

  std::unordered_map<std::pair<std::string, std::string>, double>
    trips_per_day;
  std::unordered_map<std::pair<std::string, std::string>, fhi::patch>
    commute_patches;

  std::unordered_map<std::string, std::unordered_set<std::string>>
    commute_destinations, commute_origins;

  for (size_t t = 0; t < t_max; t += dt) {
    double time_days = static_cast<double>(t)/24.0,
           dt_days = static_cast<double>(dt)/24.0;

    if (t % 24 == 0) {
      covid::report::population_report(
          time_days, patches, notable_patches, reported_compartments,
          std::cout);

      commute_destinations = {};
      commute_origins = {};
      trips_per_day = {};

      for (auto& [i, patch_i]: patches) {
        auto [mobility_from, hint_from] = mobility.trips_from(
            i, time_days, 1.0, hints_from[i]);
        for (auto& [j, trips_per_dt]: mobility_from) {
          std::pair<std::string, std::string> key(i, j);

          // create the patch if necessary
          commute_patches.try_emplace(
              key, fhi::patch::population_type());

          trips_per_day[key] = trips_per_dt;
          commute_destinations[i].insert(j);
          commute_origins[j].insert(i);
        }

        hints_from[i] = hint_from;
      }
    }

    // morning commute
    if (t % 24 == 9) {
      for (auto& [i, patch_i]: patches) {
        for (auto&& j: commute_destinations[i]) {
          size_t time_seed = std::hash<std::pair<size_t, size_t>>{}({seed, t});
          fhi::patch::population_type commuters = commuting_population(
              patch_i.population(),
              static_cast<size_t>(trips_per_day.at({i, j}) + 0.5),
              time_seed);
          commute_patches.at({i, j}).apply_delta(commuters);
          patch_i.apply_delta(-commuters);
        }
      }
    }

    // evening commute (return)
    if (t % 24 == 17) {
      for (auto& [i, patch_i]: patches) {
        for (auto&& j: commute_origins[i]) {
          fhi::patch::population_type commuters =
            commute_patches.at({j, i}).population();
          commute_patches.at({j, i}).apply_delta(-commuters);
          patch_i.apply_delta(commuters);
        }
      }
    }

    for (auto& [i, patch_i]: patches) {
      fhi::patch::population_type total_pop = patch_i.population();

      for (auto&& j: commute_origins[i])
        total_pop += commute_patches.at({j, i}).population();

      for (auto&& j: commute_origins[i]) {
        fhi::patch::population_type this_pop =
          commute_patches.at({j, i}).population();
        std::pair<std::pair<std::string, std::string>, size_t>
          seed_pair = {{i, j}, t};
        size_t commute_seed = std::hash<decltype(seed_pair)>{}(seed_pair);
        auto this_delta = commute_patches.at({j, i}).delta(
            conf, dt_days, total_pop-this_pop, commute_seed);
        commute_patches.at({j, i}).apply_delta(this_delta);
      }

      size_t patch_seed =
        std::hash<std::pair<std::string, size_t>>{}(std::make_pair(i, t));
      auto delta = patch_i.delta(
          conf, dt_days, total_pop-patch_i.population(), patch_seed);
      patch_i.apply_delta(delta);
    }
  }
}
