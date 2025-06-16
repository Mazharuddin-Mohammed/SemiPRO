#include "diffusion_solver.hpp"
#include "../../core/utils.hpp"
#include <cmath>

DiffusionSolver::DiffusionSolver() {}

Eigen::ArrayXd DiffusionSolver::simulateDiffusion(const Eigen::ArrayXd& initial_profile, double temperature, double time,
                                                 double dx, double dt) const {
  int n = initial_profile.size();
  Eigen::ArrayXd profile = initial_profile;
  Eigen::ArrayXd new_profile = profile;

  // Diffusion coefficient (cm^2/s), temperature in K
  double D = 1e-14 * std::exp(-3.0 / (8.617e-5 * temperature));
  double alpha = D * dt / (dx * dx); // Stability requires alpha < 0.5

  int steps = static_cast<int>(time / dt);
  for (int t = 0; t < steps; ++t) {
    for (int i = 1; i < n - 1; ++i) {
      new_profile[i] = profile[i] + alpha * (profile[i + 1] - 2 * profile[i] + profile[i - 1]);
    }
    profile = new_profile;
  }

  Logger::getInstance().log("Diffusion: temp=" + std::to_string(temperature) + "K, time=" + std::to_string(time) + "s");
  return profile;
}