#include "monte_carlo_solver.hpp"
#include "../../core/utils.hpp"
#include <cmath>
#include <iostream>

MonteCarloSolver::MonteCarloSolver() : rng_(std::random_device{}()) {}

Eigen::ArrayXd MonteCarloSolver::simulateImplantation(std::shared_ptr<Wafer> wafer, double energy, double dose) {
  int x_dim = wafer->getGrid().rows();
  Eigen::ArrayXd profile = Eigen::ArrayXd::Zero(x_dim);

  // Debug output
  std::cout << "DEBUG: Monte Carlo simulation starting\n";
  std::cout << "  Grid dimensions: " << x_dim << " points\n";
  std::cout << "  Wafer thickness: " << wafer->getThickness() << " μm\n";
  std::cout << "  Energy: " << energy << " keV\n";
  std::cout << "  Dose: " << dose << " cm⁻²\n";

  // Use realistic range parameters for ion implantation
  double range_mean = energy * 0.02; // Realistic range: ~1 μm for 50 keV
  double range_stdev = range_mean * 0.3; // 30% straggle
  std::normal_distribution<double> dist(range_mean, range_stdev);

  // Use a reasonable number of Monte Carlo particles (not the full dose)
  long num_ions = std::min(static_cast<long>(1e4), static_cast<long>(dose * 1e-12)); // Smaller number for faster simulation
  double dz = wafer->getThickness() / x_dim; // Grid spacing (um)

  std::cout << "  Number of ions: " << num_ions << "\n";
  std::cout << "  Grid spacing: " << dz << " μm\n";
  std::cout << "  Range mean: " << range_mean << " μm\n";
  std::cout << "  Range stdev: " << range_stdev << " μm\n";

  long ions_deposited = 0;
  for (long i = 0; i < num_ions; ++i) {
    double depth = std::max(0.0, dist(rng_)); // Depth in um
    int index = static_cast<int>(depth / dz);
    if (index < x_dim) {
      profile[index] += 1.0;
      ions_deposited++;
    }
  }

  std::cout << "  Ions deposited in wafer: " << ions_deposited << "\n";

  // Convert to concentration (cm^-3): scale by actual dose
  if (ions_deposited > 0) {
    double scaling_factor = dose / (num_ions * dz * 1e-4); // Proper scaling to get dose in cm^-2
    profile *= scaling_factor;
  }

  double max_conc = profile.maxCoeff();
  std::cout << "  Max concentration: " << max_conc << " cm⁻³\n";

  Logger::getInstance().log("Ion implantation: energy=" + std::to_string(energy) + "keV, dose=" + std::to_string(dose) + "cm^-2");
  return profile;
}