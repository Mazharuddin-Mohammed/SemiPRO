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
};

#endif // MONTE_CARLO_SOLVER_HPP