#ifndef DOPING_MANAGER_HPP
#define DOPING_MANAGER_HPP

#include "doping_interface.hpp"
#include "monte_carlo_solver.hpp"
#include "diffusion_solver.hpp"

class DopingManager : public DopingInterface {
public:
  DopingManager();
  void simulateIonImplantation(std::shared_ptr<Wafer> wafer, double energy, double dose) override;
  void simulateDiffusion(std::shared_ptr<Wafer> wafer, double temperature, double time) override;

private:
  MonteCarloSolver monte_carlo_;
  DiffusionSolver diffusion_;
  double dx_ = 1e-6; // um
  double dt_ = 1.0;  // s
};

#endif // DOPING_MANAGER_HPP