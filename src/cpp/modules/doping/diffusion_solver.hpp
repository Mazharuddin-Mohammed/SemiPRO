#ifndef DIFFUSION_SOLVER_HPP
#define DIFFUSION_SOLVER_HPP

#include "../../core/wafer.hpp"

class DiffusionSolver {
public:
  DiffusionSolver();
  Eigen::ArrayXd simulateDiffusion(const Eigen::ArrayXd& initial_profile, double temperature, double time, double dx, double dt) const;
};

#endif // DIFFUSION_SOLVER_HPP