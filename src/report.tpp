#include <iostream>
#include <unordered_map>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"  // not working (gcc bug?)
#include "magic_enum.hpp"
#pragma GCC diagnostic pop

namespace covid {
  namespace report {
    template <class PatchType, class CompartmentType>
    void age_grouped_population_report(
        double t,
        const std::unordered_map<std::string, PatchType>& patches,
        const std::vector<std::string>& notable_patches,
        const std::vector<CompartmentType>& compartments,
        std::ostream& file) {
      file << t;

      typename PatchType::population_type::item_type totals;
      for (auto&& [i, patch]: patches)
        totals += patch.population().sum();

      for (auto&& i: notable_patches) {
        auto this_total = patches.at(i).population().sum();
        for (auto&& c: compartments)
          file << "," << this_total[c];
      }

      for (auto&& c: compartments)
        file << "," << totals[c];

      file << "," << totals.sum() << "\n";
    }

    template <class PatchType, class CompartmentType>
    void population_report(
        double t,
        const std::unordered_map<std::string, PatchType>& patches,
        const std::vector<std::string>& notable_patches,
        const std::vector<CompartmentType>& compartments,
        std::ostream& file) {
      file << t;

      typename PatchType::population_type totals;
      for (auto&& [i, patch]: patches)
        totals += patch.population();

      for (auto&& i: notable_patches)
        for (auto&& c: compartments)
          file << "," << patches.at(i).population()[c];

      for (auto&& c: compartments)
        file << "," << totals[c];

      file << "," << totals.sum() << "\n";
    }

    template <class CompartmentType>
    void population_report_header(
        const std::vector<std::string>& notable_patches,
        const std::vector<CompartmentType>& compartments,
        std::ostream& file) {
      file << "time";

      for (auto&& i: notable_patches)
        for (auto&& c: compartments)
          file << "," << i << " " << magic_enum::enum_name(c);

      for (auto&& c: compartments)
        file << "," << magic_enum::enum_name(c);

      file << "," << "total" << "\n";
    }
  }  // namespace report
}  // namespace covid
