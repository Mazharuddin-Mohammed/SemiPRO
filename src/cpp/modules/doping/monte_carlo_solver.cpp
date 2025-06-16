#include "monte_carlo_solver.hpp"
#include "../../core/utils.hpp"
#include <cmath>

MonteCarloSolver::MonteCarloSolver() : rng_(std::random_device{}()) {}

Eigen::ArrayXd MonteCarloSolver::simulateImplantation(std::shared_ptr<Wafer> wafer, double energy, double dose) {
  int x_dim = wafer->getGrid().rows();
  Eigen::ArrayXd profile = Eigen::ArrayXd::Zero(x_dim);
  std::normal_distribution<double> dist(energy * 0.01, energy * 0.005); // Depth distribution (um)

  long num_ions = static_cast<long>(dose * 1e4); // Dose in cm^-2, convert to ions
  double dz = wafer->getThickness() / x_dim; // Grid spacing (um)

  for (long i = 0; i < num_ions; ++i) {
    double depth = std::max(0.0, dist(rng_)); // Depth in um
    int index = static_cast<int>(depth / dz);
    if (index < x_dim) {
      profile[index] += 1.0;
    }
  }

  profile /= (dose * 1e4 * dz); // Normalize to concentration (cm^-3)
  Logger::getInstance().log("Ion implantation: energy=" + std::to_string(energy) + "keV, dose=" + std::to_string(dose) + "cm^-2");
  return profile;
}