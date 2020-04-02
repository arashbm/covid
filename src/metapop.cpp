#include <vector>

enum class compartments : size_t {
  susceptible,  // (S)
  exposed,      // (E)
  asymptomatic, // (A)
  infected,     // (I)
  hospitalized, // (H) in ICU
  dead,         // (D)
  recovered,    // (R)
}

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
  young,   // (Y) age <= 25
  adults,  // (M) 26 <= age <= 65
  elderly, // (O) 66 <= age
}

constexpr std::array<age_groups, 3> all_age_groups = {
  age_groups::young,
  age_groups::adults,
  age_groups::elderly,
};


constexpr size_t pop_size = age_groups_count*compartments_count;


using ContactMatrixType = typename std::array<
  std::array<double, all_age_groups.size()>,
  all_age_groups.size()>;


struct config {
  double pi, eta, theta, nu, rho, chi, delta;

  double beta_infected, beta_asymptomatic;

  ContactMatrixType contact;
}

class patch {
private:
  PopulationMatrixType _population; // population[age_group][compartment]

  double age_group_force(age_groups age_group, const config& c) {
    return 0.0; // TODO
  }

  const ContactMatrixType& contact_function(const config c) {
    ContactMatrixType contact;
    contact.fill(0.0); // TODO: Use c.contact
    return contact;
  }

public:
  using PopulationMatrixType = typename std::array<
    std::array<double, all_compartments.size()>,
    all_age_groups.size()>;

  patch(PopulationMatrixType population) {
    _population = population;
  }

  const PopulationMatrixType& population() {
    return _population();
  }

  PopulationMatrixType delta_population(const config& c) {
    PopulationMatrixType delta;
    for (auto&& age_group: all_age_groups) {
      double lambda = age_group_force(age_group, c);

      delta[age_group][compartments::susceptible] =
        -lambda*_population[age_group][susceptible];

      delta[age_group][compartments::exposed] =
        +lambda*_population[age_group][susceptible]
        -c.eta*_population[compartments::exposed];

      delta[age_group][compartments::asymptomatic] =
        +(1.0-c.pi)*c.eta*_population[compartments::exposed]
        -c.rho*_population[age_group][compartments::asymptomatic];

      delta[age_group][compartments::infected] =
        +c.rho*c.eta*_population[compartments::exposed]
        -(c.theta+c.nu)*_population[age_group][compartments::infected];

      delta[age_group][compartments::hospitalized] =
        +c.theta*_population[compartments::infected]
        -(c.chi+c.delta)*_population[age_group][compartments::hospitalized];

      delta[age_group][compartments::recovered] =
        +c.rho*_population[compartments::asymptomatic]
        +c.nu*_population[compartments::infected]
        +c.chi*_population[compartments::hospitalized];

      delta[age_group][compartments::recovered] =
        +c.delta*_population[compartments::hospitalized]
    }
  }
}
