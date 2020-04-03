#ifndef INCLUDE_COVID_METAPOP_HPP_
#define INCLUDE_COVID_METAPOP_HPP_

#include <array>

namespace covid {
  enum class compartments : size_t {
    susceptible,    // (S)
    exposed,        // (E)
    asymptomatic,   // (A)
    infected,       // (I)
    hospitalized,   // (H) in ICU
    dead,           // (D)
    recovered,      // (R)
  };

  constexpr std::array<compartments, 7> all_compartments = {
    compartments::susceptible,
    compartments::exposed,
    compartments::asymptomatic,
    compartments::infected,
    compartments::hospitalized,
    compartments::dead,
    compartments::recovered,
  };

  enum class age_groups : size_t {
    young,    // (Y) age <= 25
    adults,   // (M) 26 <= age <= 65
    elderly,  // (O) 66 <= age
  };

  constexpr std::array<age_groups, 3> all_age_groups = {
    age_groups::young,
    age_groups::adults,
    age_groups::elderly,
  };




  template <class T, class I, size_t size>
  class array {
  private:
    std::array<T, size> _ar;
  public:
    array() {
      _ar.fill(T{});
    }

    array(std::initializer_list<T> args) {
      auto it = _ar.begin();
      for (auto&& i: args) {
        *it = i;
        it++;
      }
    }

    const T& operator[](I ind) const { return _ar[static_cast<size_t>(ind)]; }
    T& operator[](I ind) { return _ar[static_cast<size_t>(ind)]; }
  };

  template <typename T>
  using AgeArrayType = array<T, age_groups, all_age_groups.size()>;

  using ContactMatrixType = array<AgeArrayType<double>,
        age_groups, all_age_groups.size()>;
}  // namespace covid



#endif  // INCLUDE_COVID_METAPOP_HPP_
