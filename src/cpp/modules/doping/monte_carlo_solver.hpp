#ifndef MONTE_CARLO_SOLVER_HPP
#define MONTE_CARLO_SOLVER_HPP

#include "../../core/wafer.hpp"
#include <random>

class MonteCarloSolver {
public:
  MonteCarloSolver();
  Eigen::ArrayXd simulateImplantation(std::shared_ptr<Wafer> wafer, double energy, double dose);

private:
  mutable std::mt19937 rng_;

  // Enhanced physics calculation methods
  double calculateLSSRange(double energy, const std::string& ion, const std::string& target);
  double calculateRangeStraggle(double energy, const std::string& ion, const std::string& target);
  long calculateOptimalParticleCount(double dose, double energy, int grid_size);
  double getAtomicMass(const std::string& element);
  double getAtomicNumber(const std::string& element);
};

#endif // MONTE_CARLO_SOLVER_HPP